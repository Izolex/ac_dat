# AC-DAT

Implementation of [Aho-Corasick](http://cr.yp.to/bib/1975/aho.pdf) ([wiki](https://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_algorithm)) algorithm with [Double Array Trie](https://linux.thai.net/~thep/datrie/datrie.html) data structure in [C](https://en.wikipedia.org/wiki/C_(programming_language)).
Project provides search using [socket](https://en.wikipedia.org/wiki/Berkeley_sockets) and C library.
The automaton can store whole [Unicode](https://en.wikipedia.org/wiki/Unicode) alphabet.
User input must be [UTF8](https://en.wikipedia.org/wiki/UTF-8) encoded strings (they are encoded into [code points](https://en.wikipedia.org/wiki/Code_point) internally).
Implementation contains functions for storing the automaton in [binary file](https://en.wikipedia.org/wiki/Binary_file).
Project uses [cmake](https://en.wikipedia.org/wiki/CMake) with [pkg-config](https://en.wikipedia.org/wiki/Pkg-config). 
Example of usage can be found in [example directory](example).

Aho-Corasick (AC) is text search algorithm which uses [deterministic finite automaton](https://en.wikipedia.org/wiki/Deterministic_finite_automaton).
It's based on idea of [KMP](https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm) algorithm.
It can search text in linear time `O(n+m+c)` where n is the length of the strings, m is the length of the searched text and c is the count of matches.

Double Array Trie (DAT) is data structure which can store [trie](https://en.wikipedia.org/wiki/Trie) in two [arrays](https://en.wikipedia.org/wiki/Array_data_structure).
Storing the trie in DAT will consume less memory than "naive" implementation with [hash tables](https://en.wikipedia.org/wiki/Hash_table).
Use of the tail is optional. Without the tail it requires only one array to store dictionary.

## Automaton
Automaton is assembled from the trie, which must be assembled first.
Space complexity of the trie is much greater than one of the automaton.
Each node in the final automaton requires 4×4 bytes of memory and contains only information about DAT's base, check, AC's fail and output.
For assembling the AC automaton, [BFS](https://en.wikipedia.org/wiki/Breadth-first_search) and [DFS](https://en.wikipedia.org/wiki/Depth-first_search) algorithms are implemented.
An automaton can store a maximum of [2^31-1](https://en.wikipedia.org/wiki/2,147,483,647) (signed 32bit integer) states (tree nodes), so it can fit into (2^31-1)×16 ~= **34.4 GB of memory**.

### Tail
Tail stores the longest suffix of string which doesn't need to be branched.
Characters stored in the tail are outside the AC automaton.
Searching will be asymptotically slower when using tail because of the lack of fail and output functions for AC algorithm.
Using tail is optional.

### User data
Additional user data can be stored with the needle in the trie.
They are laying outside the trie (automaton) and their usage is optional.

### Search mode
The automaton search function requires [bitmask](https://en.wikipedia.org/wiki/Mask_(computing)) which consists of four search modes.

- *FIRST* = return only first occurrence and stop
- *EXACT* = exact match of the needle in the dictionary
- *NEEDLE* = construct and return found needle in the dictionary
- *USER_DATA* = search and return user data stored with the needle

## Socket
Repository contains app ([cmd directory](cmd)) for communication over [unix](https://en.wikipedia.org/wiki/Unix_domain_socket) or [tcp](https://en.wikipedia.org/wiki/Network_socket) [socket](https://en.wikipedia.org/wiki/Berkeley_sockets).
Handling of socket connections is build with the [libevent](https://libevent.org/) library (uses [epool](https://en.wikipedia.org/wiki/Epoll) on linux and [kqueue](https://en.wikipedia.org/wiki/Kqueue) on mac).
The worker pool supports handling of socket connections on multiple threads in parallel (via [pthreads](https://en.wikipedia.org/wiki/Pthreads)).
Count of the worker threads is provided when assembling the worker pool.

### Client
Directory [client](client) contains socket client(s) implementation.
Current implementation provides only client for [GO](https://en.wikipedia.org/wiki/Go_(programming_language)) programming language.
Repository also contains [CLI](https://en.wikipedia.org/wiki/Command-line_interface) client (see [cli directory](cli)).

### Config
Socket server has four config options listed below:

- *backlog* = number of maximum waiting connections (see [manual](https://man7.org/linux/man-pages/man2/listen.2.html))
- *clientTimeout* = maximum duration of socket connection (in seconds)
- *serverTimeout* = maximum duration of waiting for closing clients connections when shutting down server (in seconds)
- *handler* & *handlerData* = socket connection handler and his data. Default is ahoCorasickHandler with automaton, tail and user data.   

### Protocol
As described above, search in automaton supports few search modes which affects data order over socket.
Client request consists of first 4 bytes (signed 32bit int) of the length of the needle then whole needle follows.
Server response of one occurrence looks like this:
- Result of search (32bit int). When no occurrence, -1 is returned.
Positive number is success and size of the user data stored with the needle.
Zero means that the needle was successfully found, but doesn't have any user data.
- Then user data (if any) follows.
- If searching with *NEEDLE* mode, size of the needle (count of UTF8 bytes) and the needle itself.
- If searching with *FIRST* mode no other data follows.
Without this mode same message is sent for the next occurrence. The whole process is then repeated.

## Implementation
In [lib directory](lib) cmake and pkg-config configs for C library can be found.
Public headers are in [include directory](include).
Implementation uses only single library (libevent). 
Source code also contains implementation of [dynamic](https://en.wikipedia.org/wiki/Dynamic_array) doubly [linked list](https://en.wikipedia.org/wiki/Linked_list) which can perform as a [stack](https://en.wikipedia.org/wiki/Stack_(abstract_data_type)), [queue](https://en.wikipedia.org/wiki/Queue_(abstract_data_type)) or [sorted array](https://en.wikipedia.org/wiki/Sorted_array).
Supports [linear](https://en.wikipedia.org/wiki/Linear_search) and [binary](https://en.wikipedia.org/wiki/Binary_search_algorithm) search and [merge sort](https://en.wikipedia.org/wiki/Merge_sort).

### Alternatives
#### Unsigned integer as DAT index
As mentioned above, the trie index uses signed 32bit integer.
Using unsigned integer can double the number of nodes ([2^32-1](https://en.wikipedia.org/wiki/4,294,967,295)).
For doing that, the tail must be disabled and the algorithm for searching free node in the trie modified.

#### Tail as single array
Storing characters in tail requires at least n×4+4+p bytes of memory, where p is the size of pointer and n the number of characters.
The tail requires at least c×n+4+p bytes of memory, where c is the size of cell, n the number of the cells and where p is the size of pointer.
Using the tail as single character array can reduce the amount of used memory, but it really depends on dictionary.

#### Node children
When building the trie, each node keeps list of outwards transition functions (characters).
These lists speeds up solving collision in array and building AC automaton significantly, but it's costing time keeping them up-to-date.
Using hash tables or some another data structure can noticeably reduce complexity.
Another option is not keeping information about outwards transition functions but then each must be tried, so it's usefully with smaller alphabets.
Unicode 14 has 144,697 characters.

#### Not using AC output function
Output function points to the nearest end state where we can get through fail functions.
The use of the output function speeds up searching in the automaton.
It requires 4 bytes of memory per each automaton state, so not using it will reduce memory usage of the automaton by a quarter.

##
Yeah, code is not tested. I will buy you 🍺 for founded bug.
