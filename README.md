# stack-dist
Generic reuse distance analysis tool

This program reads a data reference trace on stdin and produces as output a
stack distance histogram for the reference trace.  Data references should
appear one-per-line on the input and may be any string.  Leading whitespace
is ignored as well as empty lines and lines with a '#' as the first
non-whitespace character.

Stack distance is defined for data reference X as the number of unique data
references since the last time X was referenced.  This program builds up a
hash table of data references (strings) and maintains a queue of data
references.  An entry in the queue holds a pointer to a string in the hash
table, so when we search the queue to find the time X was referenced we do
(less expensive) pointer comparison rather than string comparison.

# Building
stack-dist requires glib.  To build,
```
$ make
```

# Running
```
$ ./stack-dist.h

Stack Distance Tool - Analyze data reference traces to produce a locality histogram
  Usage: stack_dist [args] (input read on STDIN)

Options:
  -b  Number of buckets (default: 1000000)
  -h  Help
```

# Example
```
$ ./stack_dist < test_input
# Dist	     Refs	   Refs(%)	  Cum_Ref	Cum_Ref(%)
     0	        1	0.20000000	        1	0.20000000
     1	        1	0.20000000	        2	0.40000000
     2	        1	0.20000000	        3	0.60000000
     3	        1	0.20000000	        4	0.80000000
     4	        1	0.20000000	        5	1.00000000
#OVFL 	        0	0.00000000	        5	1.00000000
#INF  	        5
```

Dist - Distance
Refs - Number of references that occurred at dist distance
Refs(%) - Percent of references that occurred at dist distance
Cum_Ref - Cumulative number of references at dist or less distances
Cum_Ref(%) - Cumulative percent of references at dist or less distances
OVFL - Overflow bucket containing all distances greater than the number of buckets
INF - Infinite distance references (one per item)
