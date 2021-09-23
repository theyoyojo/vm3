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
	match(data, /([0-9]*)( |\t|\n)*(L|R|X)+.*$/, matches) 
	mktmdat = "MkTmdat(" matches[1] "," matches[3] ")"
	gsub(/[0-9]*(\t| |\n)*(L|R|X)+(\t| |\n)*$/, mktmdat, data)
	return cify_var(functab["Edge"], data)
}

function tmgenify() {

	tmgenfmt = "(struct tmgen){\n" 	\
		"\t\t.tape = %s,\n" 		\
		"\t\t.edges = MkList(Ed)(%d%s"	\
			"%s),\n" 		\
		"\t\t.vertices = %d,\n" 	\
		"\t\t.start = %d,\n" 		\
		"\t\t.acceptmask = %s,\n" 	\
		"\t\t.rejectmask = %s,\n" 	\
		"\t\t.name = %s,\n" 		\
	"\t}"

	if (length(edges) > 0) {
		prefix = ",\n"
		for (i in edges) {
			if (i < length(edges) - 1) {
				elist = elist "\t\t" edges[i] ",\n"
			} else {
				elist = elist "\t\t" edges[i]
			}
		}
	} else {
		prefix = ""
		elist = ""
	}

	return sprintf(tmgenfmt,
		inputs["Tape"],
		length(edges),
		prefix,
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
		"\tTm_setverbose(t, %s);\n"		\
		"\tret = Tm_run(t);\n\n"		\
		"\treturn ret;\n}\n"

	return sprintf(main, tmgen, inputs["Verbose"])
}

BEGIN {
	RS=";"
	inputs["Tape"] = ""
	inputs["Accept"] = ""
	inputs["Reject"] = ""
	inputs["Start"] = ""
	inputs["Vertices"] = ""
	inputs["Verbose"] = "0"
	# declare edges as empty array
	delete edges[0]
}

/^.*#.*/  {}

$1 ~ /Tape|Accept|Reject/ { inputs[$1] = cify_var(getfunc($1), $0) }

$1 ~ /Start|Vertices/ { inputs[$1] = $2 }

$1 ~ /Verbose/ { inputs[$1] = "1" }

$1 ~ /Name/ {
	gsub(/^( |\n|\t)*Name( |\t|\n)*/, "", $0);
	gsub(/\n/, "\\n", $0)
	inputs["Name"] = "\"" $0 "\"";
}

$1 ~ /Edge/ { edges[length(edges)] = edgify($0) }

END {
	printf "%s\n", mainify(tmgenify())
}
