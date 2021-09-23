# vm3
yet a third

To build and run the example tm in `templ.tm`:

```bash
make templ.tmb
./templ.tmb
```

Take a look at the file to get an idea of how to write arbitrary turing machines:

```

# Template Turing Machine definition

# Brief definition of language:
# <empty> ~ /^$/
# <text> ~ /^.*$/
# <int> ~ /^[0-9]*$/
# MaxVert := <int>
# <vert> := 0 < <int> <= MaxVert
# <verts> := <verts> <vert> | <vert>
# <verts?> := <verts> | <empty>
# <op> ~ /^(L|R|X)+$/
#;

# Name <text>
# Specify the name of the turing machine in ascii;
Name an example
of a turing machine;

# Verbose <empty>
# (optional) Specify that the machine should print more output to stdin;
Verbose;

# Vertices $MaxVert
# Specify the maximum vertex ID. Defines all vertices 1..$MaxVert as valid;
Vertices 3;

# Tape <ints>
# Specify the initial state of the machine tape with an arbitrary list of integers;
Tape 3 1;

# Start <vert>
# Specify the start state for the state machine;
Start 1;

# Accept <verts?>
# Specify a list of zero or more accepting states that halt the turing machine;
Accept 2;

# Reject <verts?>
# Specify a list of zero or more rejecting states that halt the turing machine;
Reject;

# Edge <vert> <vert> <int> <int> <op>
# Specify zero or more edges
# NOTE: writing zeros to the tape is not implemented;
Edge 1 1 3 4 R;
Edge 1 2 1 0 L;
Edge 1 3 2 1 R;
```
