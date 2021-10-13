module tb;
  logic clock = 0;
  logic arst;
  initial forever begin
    clock = #5 !clock;
  end
  initial begin
    arst = 1;
    #10; 
    arst = 0;
    #10;
    arst = 1;
    #150;
    $finish;
  end
  initial begin
    $dumpfile("single-wait.vcd");
    $dumpvars;
  end
  test_sc_module_1 dut(
    .clk(clock),
    .reset(arst)
  );
endmodule
