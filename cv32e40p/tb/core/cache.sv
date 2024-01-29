module cache (
    input  logic        data_req_i, // Request received from core 
    input  logic [31:0] data_addr_i, // Address sent by core
    input  logic        data_we_i, // Specify if load or store instruction initiated by core
    input  logic [3:0]  data_be_i,
    input  logic	data_en, // Cache enable
    input  logic [31:0] data_wdata_i, // Store instr 
    input  logic [31:0] data_rdata_i,// Data to send to the core 
    input  logic 	clk,
    input  logic        data_gnt_i,
    input  logic        data_rvalid_i, // Valid value sent
    output logic 	data_req_o, // Data request to memory
    output logic [31:0] data_rdata_o,// Data to send to the core 
    output logic [31:0] data_addr_o, // When requesting data from memory
    output logic        data_rvalid_o, // Valid value sent
    output logic [3:0]  data_be_o,
    output logic	data_we_o,
    output logic        init_cache = 1'b1,
    output logic        data_gnt_o, // grant signal to core
    output logic        data_mux_o = 1'b0 
);

reg [31:0] cache_data [0:1023];
reg [31:0] cache_tag [0:1023];
reg [31:0] cache_valid [0:1023];
reg [9:0]  cache_fifo_head; // Counter for FIFO replacement policy


always_comb begin

    automatic integer index = data_addr_i[9:0];
    data_gnt_o = data_req_i ? 1'b1 : 1'b0;
    if (init_cache) begin
    for (int i = 0; i <= 1023; i++) begin
    cache_data[i] = 32'h0000_0000;
    cache_tag[i] = 32'h0000_0000;
    cache_valid[i] = 32'h0000_0000;
    end
    init_cache = 1'b0;
    cache_fifo_head = 10'b0000_0000; // Initialize cache_fifo_head to 0
    end
    data_addr_o <= data_addr_i; // Address sent by core 
    data_we_o <= 1'b0; // Only reading 
    data_be_o <= data_be_i; // Assume a word-aligned access
    if (data_en) begin 
    if (data_we_i == 1'h0 ) begin // LOAD INSTRUCTION 
    if (cache_valid[index] && cache_tag[index] == data_addr_i[31:10]) begin // HIT, data is present in cache
    
      if(data_rvalid_i) begin
      data_rdata_o <= data_rdata_i;
      data_mux_o<=1'b1;
      end else begin
      data_rdata_o <= cache_data[index]; // Data Sent
      data_mux_o<=1'b1;
      end
      data_rvalid_o <= 1'b1; // Validate it

      // Print cache hit message
      if(data_rvalid_o) begin
      $display("Load transaction hit : Word read: 0x%08x, Valid_data:0x%02x ",cache_data[index], data_rvalid_o);
      $display("Data is present in cache at address: 0x%08x",data_addr_i[31:0]);
      end
    end	else begin // Load MISS (memory data request)
    
    data_rvalid_o <= 1'b0;

    // Memory paquet request
    if(!data_gnt_i) begin
    data_req_o <= 1'b1;
    end else begin 
    data_req_o <= 1'b0;
    end
    data_mux_o<=1'b0;
    //
    if(data_rvalid_i) begin  // Valid data sent by memory 
			  cache_data[index] <= data_rdata_i; // Store it in the cache
			  cache_tag[index] <= data_addr_i[31:10]; // Store the tag
			  cache_valid[index] <= 1'b1; // Validate the cache line

                          $display("Load transaction Miss (from memory): data read: 0x%08x",data_rdata_i);
			  // Return the data to the processor
			  data_rdata_o <= data_rdata_i; 
			
			  data_rvalid_o <= 1'b1; // Data is now valid
                          end
    end
    end else begin  // STORE TRANSACTION 
          // Cache line is empty, write data
          cache_data[index] <= data_wdata_i;
          cache_tag[index] <= data_addr_i[31:10];
          cache_valid[index] <= 1'b1;
         $display("Store transaction: data written: 0x%08x at index: %d",cache_data[index], index);
    end
end
end
endmodule

