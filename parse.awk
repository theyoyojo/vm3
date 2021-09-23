#!/bin/awk -f

function cify(fn, args) {

	out = fn "("
	for (dat in args) {
		if (dat == 1) {
			out = out args[dat]
		} else {
			out = out ", " args[dat]
		}
	}
	out = out ")"
	return out
}

function cify_var(fn, defs) {
	split(defs, args, " ")
	args[1] = (length(args)-1)
	return cify(fn, args)
}

function getfunc(id) {
	functab["Tape"] = "MkTp"
	functab["Accept"] = "Mkmask"
	functab["Reject"] = "Mkmask"
	functab["Edge"] = "MkEd"

	return functab[id]
}

function edgify(data) {
	# gsub(/^( |\n|\t)*Edge( |\t|\n)*/, "", data);
	# print data
	match(data, /([0-9]*)( |\t|\n)*(L|R|X)+.*$/, matches) 
	# printf "%s %s\n", matches[1], matches[3]
	# split(data, args, " ")
	mktmdat = "Mktmdata(" matches[1] "," matches[3] ")"
	gsub(/[0-9]*(\t| |\n)*(L|R|X)+(\t| |\n)*$/, mktmdat, data)
	# for (i in args) {
	# 	defs = args[i] " "
	# }
	return cify_var(functab["Edge"], data)
}

function tmgenify() {

	tmgenfmt = "(struct tmgen){\n" 	\
		"\t\t.tape = %s,\n" 		\
		"\t\t.edges = MkList(Ed)(%d,\n"	\
			"%s),\n" 		\
		"\t\t.vertices = %d,\n" 	\
		"\t\t.start = %d,\n" 		\
		"\t\t.acceptmask = %s,\n" 	\
		"\t\t.rejectmask = %s,\n" 	\
		"\t\t.name = %s,\n" 		\
	"\t}"

	for (i in edges) {
		if (i < length(edges) - 1) {
			elist = elist "\t\t" edges[i] ",\n"
		} else {
			elist = elist "\t\t" edges[i]
		}
	}

	return sprintf(tmgenfmt,
		inputs["Tape"],
		length(edges),
		elist,
		inputs["Vertices"],
		inputs["Start"],
		inputs["Accept"],
		inputs["Reject"],
		inputs["Name"])


}

function mainify(tmgen) {
	main="#include \"tm.h\"\n\n" 			\
		"int main(void) {\n"			\
		"\tint ret;\n"				\
		"\tstruct tmgen tg = %s;\n"		\
		"\tTm * t = Tm_gen(&tg);\n"		\
		"\tret = Tm_run(t);\n\n"		\
		"\treturn 0;\n}\n"

	return sprintf(main, tmgen)
}

BEGIN {
	# print foo();
	RS=";"
	inputs["Tape"] = ""
	inputs["Accept"] = ""
	inputs["Reject"] = ""
	inputs["Start"] = ""
	inputs["Vertices"] = ""
	# declare edges as empty array
	delete edges[0]
}

/^.*#.*/  {}

$1 ~ /Tape|Accept|Reject/ { inputs[$1] = cify_var(getfunc($1), $0) }

$1 ~ /Start|Vertices/ { inputs[$1] = $2 }

$1 ~ /Name/ {
	gsub(/^( |\n|\t)*Name( |\t|\n)*/, "", $0);
	inputs["Name"] = "\"" $0 "\"";
}

$1 ~ /Edge/ { edges[length(edges)] = edgify($0) }

END {
	# for (i in inputs) {
	# 	printf "%s: %s\n", i, inputs[i]
	# }
	# for (i in edges) {
	# 	printf "E%d: %s\n", i, edges[i]
	# }

	printf "%s\n", mainify(tmgenify())
}
