`include "RegisterEnum.v"

module RegisterFile(input logic clk,
                    input logic reset,
                    input logic is_8_bit,
                    // Read port.
                    input logic [2:0] rd_sel[2],
                    output logic [15:0] rd_val[2],
                    // Write port.
                    input logic [2:0] wr_sel,
                    input logic [15:0] wr_val,
                    input logic wr_en);

reg [15:0] gprs[8];

wire wr_sel_low_byte = ~wr_sel[2];
wire [2:0] wr_8_bit_sel = {1'b0, wr_sel[1:0]};

integer i;

always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
        for (i = 0; i < 8; ++i)
            gprs[i] <= 16'b0;
    end else if (wr_en) begin
        if (is_8_bit) begin
            if (wr_sel_low_byte)
                gprs[wr_8_bit_sel][7:0] <= wr_val[7:0];
            else
                gprs[wr_8_bit_sel][15:8] <= wr_val[7:0];
        end else begin
            gprs[wr_sel] <= wr_val;
        end
    end
end

genvar rd_port;

generate
for (rd_port = 0; rd_port < 2; ++rd_port) begin: read_port
    wire rd_sel_low_byte = ~rd_sel[rd_port][2];
    wire [2:0] rd_8_bit_sel = {1'b0, rd_sel[rd_port][1:0]};
    wire bypass = wr_en && wr_sel == rd_sel[rd_port];

    always_ff @(posedge clk) begin
        if (is_8_bit)
            rd_val[rd_port] <= bypass ? {8'b0, wr_val[7:0]} :
                {8'b0, rd_sel_low_byte ?
                    gprs[rd_8_bit_sel][7:0] : gprs[rd_8_bit_sel][15:8]};
        else
            rd_val[rd_port] <= bypass ? wr_val :
                gprs[rd_sel[rd_port]];
    end
end
endgenerate

`ifdef verilator
export "DPI-C" function get_gpr;

function [15:0] get_gpr;
    input int regnum;
    get_gpr = gprs[regnum];
endfunction

export "DPI-C" function set_gpr;

function set_gpr;
    input int regnum;
    input int val;
    gprs[regnum] = val[15:0];
    set_gpr = 0;
endfunction
`endif

endmodule
