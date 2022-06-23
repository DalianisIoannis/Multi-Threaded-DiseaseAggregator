# Multi-Threaded-DiseaseAggregator

This project contains an implementation of a multithreaded, network-based application, with a server and an arbitrary number of clients.


The server starts listening to a given port and corresponds accordingly. Each client connects to the server and then creates a threadpool with some threads to deal with the round buffer. Each client also uses a port for replies from the server and the other clients.
