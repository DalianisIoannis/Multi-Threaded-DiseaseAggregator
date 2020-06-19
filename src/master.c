#include "../headers/general.h"
#include "../headers/workers.h"
#include "../headers/countryList.h"
#include "../headers/pipes.h"
#include "../headers/fatherFunctions.h"
#include "../headers/statistics.h"
#include "../headers/signals.h"

int main(int argc, char **argv)
{
    // argv[0] ./master
    // argv[1] -w
    // argv[2] numWorkers
    // argv[3] –b
    // argv[4] bufferSize
    // argv[5] –s
    // argv[6] serverIP
    // argv[7] –p
    // argv[8] serverPort
    // argv[9] –i
    // argv[10] input_dir

    if(argv[10]==NULL){
        fprintf(stderr, "Command must be in form: ./master –w numWorkers -b bufferSize –s serverIP –p serverPort -i input_dir!\n");
        exit(1);
    }

    int bufferSize = atoi(argv[4]);
    int numWorkers = atoi(argv[2]);
    char* serverIP = strdup(argv[6]);
    int serverPort = atoi(argv[8]);

    char *input_dir = strdup(argv[10]);

    workerDataNode* WorkersArr = NULL;
    if ( (WorkersArr = malloc(numWorkers*sizeof(workerDataNode*))) == NULL ) {
        perror("WorkersArr malloc");
        exit(1);
    }

    termination = 0;
    sigkill = 0;
    struct sigaction *act = malloc(sizeof(struct sigaction));
    HandlerInit(act, handler);

    pid_t pid = 0;
    for (int i=0; i < numWorkers; i++) {
        pid = fork();

        if (pid < 0) {
            perror("Fork");
            exit(1);
        }

        else if (pid == 0) {    // child

            int rfd, wfd;

            char* fiforead = malloc( (strlen("./pipeFiles/reader_des")+12) );
            sprintf(fiforead, "./pipeFiles/reader_des%d", getpid());

            char *fifowrite = malloc((strlen("./pipeFiles/writer_des")+12));
            sprintf(fifowrite, "./pipeFiles/writer_des%d", getpid());

            if ((wfd = open(fiforead, O_WRONLY)) == -1) {
                perror("open wfd");

            }
            if ((rfd = open(fifowrite, O_RDONLY)) == -1) {
                perror("open rfd");
            }

            WorkerRun(input_dir, bufferSize, rfd, wfd, numWorkers);

            for(int j=0; j < i; j++) {

                emptyworkerNode(&WorkersArr[j]);

            }

            free(WorkersArr);
            free(fiforead);
            free(fifowrite);
            free(input_dir);
            free(act);
            free(serverIP);
            
            exit(0);
        }

        else {  // father
            if ((WorkersArr[i] = makeWorkerArCell(pid, bufferSize)) == NULL)
            {
                perror("makeWorkerArCell failed\n");
            }
        }
    }

    // serverIP and serverPort
    if(sendIPandPortToWorkers(WorkersArr, numWorkers, bufferSize, serverIP, serverPort)!=0) {
        perror("IP and Port\n");
        return -1;
    }
    

    // countries for every pid
    CountryList* countriesListArray = malloc(numWorkers*sizeof(CountryList));
    for (int i=0; i < numWorkers; i++) {
        countriesListArray[i]=initcountryList();
    }

    sendCountriesToWorkers(WorkersArr, input_dir, numWorkers, bufferSize, countriesListArray);

    // FatherQuerries(WorkersArr, numWorkers, bufferSize, countriesListArray);

    for(int i=0; i < numWorkers; i++) {
        wait(NULL);
    }
    
    for (int i=0; i < numWorkers; i++) {
        emptycountryList( &(countriesListArray[i]) );
        free(countriesListArray[i]);
    }
    free(countriesListArray);

    for(int i=0; i < numWorkers; i++) {

        unlink(WorkersArr[i]->fifoRead);
        unlink(WorkersArr[i]->fifoWrite);

        close( WorkersArr[i]->fdRead );
        close( WorkersArr[i]->fdWrite );

        emptyworkerNode(&(WorkersArr[i]));
    
    }
    free(WorkersArr);
    free(input_dir);
    free(act);
    free(serverIP);
    return 0;
}