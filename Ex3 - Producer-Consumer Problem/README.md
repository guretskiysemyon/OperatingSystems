### Explanation:
The goal of this assignment is to gain experience with concurrent programming and synchronization mechanisms in C. Students are asked to implement a news broadcasting scenario with multiple threads that produce, process, and display news stories represented as simple text strings.

The system consists of 4 main components:

- Producers: Generate news stories as strings in a particular format and add them to their own private queues.
- Dispatcher: Receives stories from the producers' queues in a round-robin fashion, classifies them by type, and adds them to dispatcher queues by type.
- Co-Editors: Receive stories from the dispatcher queues, "edit" them by blocking for a short time, and pass them to a shared queue.
- Screen Manager: Displays the edited stories from the shared queue to stdout.

You can see the assignment text in Ex3.pdf and the scheme of communication and roles in this problem.

The key challenges are to:

- Implement thread-safe bounded buffers for the producer queues and shared queue using semaphores and mutexes.
- Coordinate the producer, dispatcher, co-editor, and screen manager threads.
- Handle synchronization issues like blocking and signaling to display the stories in order.
- The configuration is specified in a text file indicating the number of producers, messages per producer, and queue sizes. The program takes the config file as input, initializes the components based on it, runs the simulation, and outputs the messages in order to stdout.

The goals are to gain exposure to pthreads, mutexes, semaphores, bounded buffers, producer-consumer coordination, and overall thread synchronization issues in C. Careful design before implementation is emphasized.oals are to gain exposure to pthreads, mutexes, semaphores, bounded buffers, producer-consumer coordination, and overall thread synchronization issues in C. Careful design before implementation is emphasized.


### Running
1. Execute the ```make``` command.
2. Modify the "conf.txt" file according to your preferences, while ensuring that it maintains the general format specified in the Ex3.pdf document. You can find the role of each line in the document.
3. Run the ```./Consumer_Producer.out conf.txt``` command. This will display the program's output on the screen.
4. Optionally, you can execute ```make clean``` if desired.
