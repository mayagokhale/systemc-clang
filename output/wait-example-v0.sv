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
  logic signed[31:0] _scclang_state_single_wait;
  logic signed[31:0] _scclang_next_state_single_wait;
  logic signed[31:0] _scclang_wait_counter_single_wait;
  logic signed[31:0] _scclang_next_wait_counter_single_wait;
  logic signed[31:0] _scclang_wait_next_state_single_wait;
  logic signed[31:0] _scclang_save_wait_next_state_single_wait;
  // Thread: single_wait
  always @(posedge clk or negedge reset) begin: single_wait_state_update
    if ((reset) == (0)) begin
      _scclang_state_single_wait <= (0);
      _scclang_wait_next_state_single_wait <= (0);
      _scclang_wait_counter_single_wait <= (0);
    end else begin
      _scclang_state_single_wait <= (_scclang_next_state_single_wait);
      _scclang_wait_counter_single_wait <= (_scclang_next_wait_counter_single_wait);
      _scclang_wait_next_state_single_wait <= (_scclang_save_wait_next_state_single_wait);
    end

  end
  always @(*) begin: single_wait
    // default case to prevent Latch generation
    // we might need generate default assignments for all signals written in
    // single_wait
    _scclang_next_state_single_wait =  _scclang_state_single_wait;
    _scclang_next_wait_counter_single_wait = _scclang_wait_counter_single_wait;
    _scclang_save_wait_next_state_single_wait = _scclang_wait_next_state_single_wait;
    case(_scclang_state_single_wait)
      0: begin
        if (1) begin
          k = 1;
          _scclang_next_wait_counter_single_wait = 4;
          _scclang_save_wait_next_state_single_wait = 1;
          _scclang_next_state_single_wait = 2;
        end
      end
      1: begin
        k = 2;
        if (1) begin
          k = 1;
          _scclang_next_wait_counter_single_wait = 4;
          _scclang_save_wait_next_state_single_wait = 1;
          _scclang_next_state_single_wait = 2;
        end
      end
      2: begin
        // an alternative
        // _scclang_next_wait_counter_single_wait = _scclang_wait_counter_single_wait - 1;
        _scclang_next_wait_counter_single_wait--;
        if ((_scclang_next_wait_counter_single_wait) == (0)) begin
          _scclang_next_state_single_wait = _scclang_wait_next_state_single_wait;
        end
      end
    endcase
  end
endmodule