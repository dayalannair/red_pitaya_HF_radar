`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: UCT
// Engineer: Dayalan Nair
// 
// Create Date: 21.09.2021 09:59:36
// Design Name: 
// Module Name: phase_inc_v3
// Project Name: 
// Target Devices: Any
// Tool Versions: 
// Description: 
// Provides incremented phase to a DDS Compiler to generate a chirp function for use in HF Radar
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments: Generates chirp only. Optimised for speed. Further optimisation by incl. LUT is possible
// 
//////////////////////////////////////////////////////////////////////////////////


module pinc_FSM(
    input clk,
    input init,
    input gen,
    input[31:0] delay,
    output [15:0] pinc,
    output m_axis_tvalid
);
    reg [15:0] pinc_array [500:0]; //set this based on pinc_max - pinc min
    parameter[0:0] OFF = 0;
    parameter[0:0] ON = 1;
    reg state = OFF;
    integer i;
    integer j;
    integer k;
    reg valid;
    //values pre-calculated to improve performance
    parameter [15:0] pinc_min= 5086;
    parameter [15:0] pinc_max= 5400;
    parameter [15:0] num_increments = pinc_max-pinc_min;
    reg [15:0] pinc_out;
    //loop for 7000 should give 14us for each pinc
    reg[31:0] dly = 26;
    assign  pinc = pinc_out;
    assign m_axis_tvalid = valid;
    always@(posedge init) begin
        dly = delay;
        for (i = 0 ; i <= num_increments ; i = i+1)
            begin
                pinc_array[i] = pinc_min + i;
            end
    end
    always@ (posedge clk) begin
        case(state)
            OFF: begin
                if (gen) state = ON;
                pinc_out = 0;
                valid = 0;
                j  = 0;
                k = 0;
            end
            ON: begin
                valid = 1;
                if (k<delay) begin
                    k = k + 1;
                    pinc_out  = pinc_array[j];
                end
                else if (j >= num_increments) state = OFF;
                    
                else begin
                    j = j + 1;
                    k = 0;
                end
            end
        endcase
    end
endmodule
