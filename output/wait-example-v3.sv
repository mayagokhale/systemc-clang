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
  logic signed[31:0] _scclang_single_wait_state;
  logic signed[31:0] _scclang_single_wait_state_next;
  logic signed[31:0] _scclang_single_wait_wait_counter;
  logic signed[31:0] _scclang_single_wait_wait_counter_next;
  logic signed[31:0] _scclang_single_wait_state_after_wait;
  logic signed[31:0] _scclang_single_wait_state_after_wait_next;
  // Thread: single_wait
  always @(posedge clk or negedge reset) begin: single_wait_state_update
    if ((reset) == (0)) begin
      _scclang_single_wait_state <= (0);
      _scclang_single_wait_state_after_wait <= (0);
      _scclang_single_wait_wait_counter <= (0);
    end else begin
      _scclang_single_wait_state <= (_scclang_single_wait_state_next);
      _scclang_single_wait_wait_counter <= (_scclang_single_wait_wait_counter_next);
      _scclang_single_wait_state_after_wait <= (_scclang_single_wait_state_after_wait_next);
    end

  end
  always @(*) begin: single_wait
    // default case to prevent Latch generation
    // we might need generate default assignments for all signals written in
    // single_wait
    _scclang_single_wait_state_next =  _scclang_single_wait_state;
    _scclang_single_wait_wait_counter_next = _scclang_single_wait_wait_counter;
    _scclang_single_wait_state_after_wait_next = _scclang_single_wait_state_after_wait;
    case(_scclang_single_wait_state)
      0: begin
        if (1) begin
          k = 1;
          _scclang_single_wait_wait_counter_next = 4;
          _scclang_single_wait_state_after_wait_next = 1;
          _scclang_single_wait_state_next = 2;
        end
      end
      1: begin
        k = 2;
        if (1) begin
          k = 1;
          _scclang_single_wait_wait_counter_next = 4;
          _scclang_single_wait_state_after_wait_next = 1;
          _scclang_single_wait_state_next = 2;
        end
      end
      2: begin
        // an alternative
        // _scclang_single_wait_wait_counter_next = _scclang_single_wait_wait_counter - 1;
        _scclang_single_wait_wait_counter_next--;
        if ((_scclang_single_wait_wait_counter_next) == (0)) begin
          _scclang_single_wait_state_next = _scclang_single_wait_state_after_wait;
        end
      end
    endcase
  end
endmodule