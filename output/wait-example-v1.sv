module DUT_sc_module_0 (
);
  logic signed[63:0] double_sig;
  logic signed[31:0] sig1;
  logic signed[31:0] others;
  test_sc_module_1 testing(
    .in1(sig1),
    .out1(sig1)
  );
  always @(*) begin
  end

endmodule
module test_sc_module_1 (
  input logic [0:0] clk,
  input logic signed[31:0] in1,
  input logic [0:0] reset,
  output logic signed[31:0] out1
);
  logic signed[31:0] k;
  logic signed[31:0] _scclang_state_single_wait_q;
  logic signed[31:0] _scclang_state_single_wait_d;
  logic signed[31:0] _scclang_wait_counter_single_wait_q;
  logic signed[31:0] _scclang_wait_counter_single_wait_d;
  logic signed[31:0] _scclang_wait_next_state_single_wait_q;
  logic signed[31:0] _scclang_wait_next_state_single_wait_d;
  // Thread: single_wait
  always @(posedge clk or negedge reset) begin: single_wait_state_update
    if ((reset) == (0)) begin
      _scclang_state_single_wait_q <= (0);
      _scclang_wait_next_state_single_wait_q <= (0);
      _scclang_wait_counter_single_wait_q <= (0);
    end else begin
      _scclang_state_single_wait_q <= (_scclang_state_single_wait_d);
      _scclang_wait_counter_single_wait_q <= (_scclang_wait_counter_single_wait_d);
      _scclang_wait_next_state_single_wait_q <= (_scclang_wait_next_state_single_wait_d);
    end

  end
  always @(*) begin: single_wait
    // default case to prevent Latch generation
    // we might need generate default assignments for all signals written in
    // single_wait
    _scclang_state_single_wait_d =  _scclang_state_single_wait_q;
    _scclang_wait_counter_single_wait_d = _scclang_wait_counter_single_wait_q;
    _scclang_wait_next_state_single_wait_d = _scclang_wait_next_state_single_wait_q;
    case(_scclang_state_single_wait_q)
      0: begin
        if (1) begin
          k = 1;
          _scclang_wait_counter_single_wait_d = 4;
          _scclang_wait_next_state_single_wait_d = 1;
          _scclang_state_single_wait_d = 2;
        end
      end
      1: begin
        k = 2;
        if (1) begin
          k = 1;
          _scclang_wait_counter_single_wait_d = 4;
          _scclang_wait_next_state_single_wait_d = 1;
          _scclang_state_single_wait_d = 2;
        end
      end
      2: begin
        // an alternative
        // _scclang_wait_counter_single_wait_d = _scclang_wait_counter_single_wait_q - 1;
        _scclang_wait_counter_single_wait_d--;
        if ((_scclang_wait_counter_single_wait_d) == (0)) begin
          _scclang_state_single_wait_d = _scclang_wait_next_state_single_wait_q;
        end
      end
    endcase
  end
endmodule