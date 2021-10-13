a.out: wait-example.tpl.sv generate.py tb.sv
	python generate.py
	iverilog -g2012 -s tb tb.sv output/wait-example-v0.sv

sim: a.out
	vvp ./a.out
