# Turing machine definition;

# <text> ~ /^.*$/;
# <int> ~ /^[0-9]*$/;
# MaxVert := <int>;
# <vert> := 0 < <int> <= MaxVert;
# <verts> := <verts> <vert> | <vert>;
# <op> ~ /^(L|R|X)+$/;

# Name <text>;
Name hello there;

# Verbose;
Verbose;

# Vertices $MaxVert;
Vertices 3;

# Tape <ints>;
Tape 3 1;

# Start <vert>;
Start 1;

# Accept <vert>;
Accept 2;

# Reject <vert>;
Reject ;

# TODO: writing zeros;
# Edge <vert> <vert> <int> <int> <op>;
Edge 1 1 3 4 R;
Edge 1 2 1 0 L;
Edge 1 3 2 1 R;
