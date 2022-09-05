# AC-DAT

Implementation of [Aho-Corasick](http://cr.yp.to/bib/1975/aho.pdf) ([wiki](https://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_algorithm)) algorithm with [Double Array Trie](https://www.co-ding.com/assets/pdf/dat.pdf) data structure in [C](https://en.wikipedia.org/wiki/C_(programming_language)).
Accepts [UTF8](https://en.wikipedia.org/wiki/UTF-8) encoded strings.
Characters are internally encoded into [Unicode](https://en.wikipedia.org/wiki/Unicode) [code points](https://en.wikipedia.org/wiki/Code_point).
Implementation contains functions for storing the automaton in [Binary file](https://en.wikipedia.org/wiki/Binary_file).
Example of usage can be found in src/main.c file.

Aho-Corasick (AC) is text search algorithm which uses [Deterministic finite automaton](https://en.wikipedia.org/wiki/Deterministic_finite_automaton) (DFA, also [FSA](https://en.wikipedia.org/wiki/Finite-state_machine)).
It's based on idea of [KMP](https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm) algorithm.
It can search text in linear time `O(n+m+c)` where n is the length of the strings, m is the length of the searched text and c is the count of matches.

Double Array Trie (DAT) is data structure which can store [Trie](https://en.wikipedia.org/wiki/Trie) in two [Arrays](https://en.wikipedia.org/wiki/Array_data_structure).
Storing the trie in DAT will consume less memory than in [Hash tables](https://en.wikipedia.org/wiki/Hash_table).
The use of the tail is optional. Without the tail it requires only one array to store dictionary.

## Implementation
### Automaton
Since the automaton is assembled from the trie, it must be assembled first.
Space complexity of the trie is greater than the automaton.
Each node in the trie requires at least 8+p+4×n+l bytes of memory where p is the size of pointer, n is the number of outwards transition functions (characters) and l the size of the list.
Each node in the final automaton requires 4×4 bytes of memory and contains only information about DAT's base, check and AC's fail, output.
For assembling the AC automaton the [BFS](https://en.wikipedia.org/wiki/Breadth-first_search) and [DFS](https://en.wikipedia.org/wiki/Depth-first_search) algorithms are implemented.
Character code points are used as transition functions and the final automaton doesn't contain information about them.
An automaton can store a maximum of [2^31-1](https://en.wikipedia.org/wiki/2,147,483,647) (signed 32bit integer) states (tree nodes).
The fully allocated automaton can fit into (2^31-1)×16+4+p ~= **34.4 GB of memory**.

### Tail
The tail stores the longest suffix of string which doesn't need to be branched.
Depending on dictionary it can significantly reduce memory usage of DAT.
Characters stored in the tail are outside the AC automaton.
Searching will be asymptotically slower when using tail because of the lack of fail and output functions for AC algorithm.
If algorithm encounters a mismatch in tail characters, it will always fail back to the state before the transition to the tail.
Using tail is optional.

### List
The list is implementation of [dynamic](https://en.wikipedia.org/wiki/Dynamic_array) doubly [Linked list](https://en.wikipedia.org/wiki/Linked_list) which can perform as [Stack](https://en.wikipedia.org/wiki/Stack_(abstract_data_type)), [Queue](https://en.wikipedia.org/wiki/Queue_(abstract_data_type)) or [Sorted array](https://en.wikipedia.org/wiki/Sorted_array).
Supports [Linear](https://en.wikipedia.org/wiki/Linear_search) and [Binary](https://en.wikipedia.org/wiki/Binary_search_algorithm) search and [Merge sort](https://en.wikipedia.org/wiki/Merge_sort).

## Implementation alternatives
### Unsigned integer as DAT index
As mentioned above, the trie index uses signed 32bit integer.
This is to indicate that the node continues in tail and double linking of free nodes which speeds up solving collision in array.
Using unsigned integer can double the number of nodes ([2^32-1](https://en.wikipedia.org/wiki/4,294,967,295)).
For doing that, the tail must be disabled and the algorithm for searching free node modified.

### Tail as single array
Storing characters in tail requires at least n×4+4+p bytes of memory, where p is the size of pointer and n the number of characters.
The tail requires at least c×n+4+p bytes of memory, where c is the size of cell, n the number of the cells and where p is the size of pointer.
Using the tail as single character array can reduce the amount of used memory, but it really depends on dictionary.

### Hash table for node children
When building the trie, each node keeps list of outwards transition functions (characters).
These lists speeds up solving collision in array and building AC automaton dramatically, but it's costing time keeping them up-to-date.
Inserting and deleting in sorted array has time complexity `θ(n)`.
Using hash tables can reduce time complexity to `θ(1)`, however efficient algorithm for retrieving all values must be found.
Another option is not keeping information about outwards transition functions then each must be tried, so it's usefully with smaller alphabets.
Unicode 14 has 144,697 characters.

### Not using AC output function
Output function points to the nearest end state where we can get through fail functions.
The use of the output function speeds up searching in the automaton.
It requires 4 bytes of memory per each automaton state.
Not using this function can reduce memory usage of the fully allocated automaton to (2^31-1)×12+4+p ~= 25.8 GB of memory.

##
I will buy you 🍺 for founded bug.

**WIP**
