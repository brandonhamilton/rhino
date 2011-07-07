                    ----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    21:56:34 02/01/2011 
-- Design Name: 
-- Module Name:    rhino_proc_intrfc_top - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description:     Emulates a read-only memory on the FPGA, via the FPGA-processor interface.
--                      The returned value is identical to the lower 16-bit of the address.
--
-- Dependencies: 
--
-- Revision: $Rev: 189 $ $Id: gpmc_test.vhd 189 2011-07-01 16:17:20Z al98277 $
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;  -- not a standard library

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
library UNISIM;
use UNISIM.VComponents.all;


------------------------------------------------------------------------------------
-- Declare input and output pins for rhino_proc_intrfc_top
------------------------------------------------------------------------------------

entity gpmc_test_top is
port
(
    -- FPGA-processor interface pins
   gpmc_a           : in std_logic_vector(10 downto 1);
   gpmc_d           : inout std_logic_vector(15 downto 0);
   gpmc_clk_i       : in std_logic;
   gpmc_n_cs        : in std_logic_vector(6 downto 0);
   gpmc_n_we        : in std_logic;
   gpmc_n_oe        : in std_logic;
   gpmc_n_adv_ale   : in std_logic;
   gpmc_n_wp        : in std_logic;
   gpmc_busy_0      : out std_logic;
   gpmc_busy_1      : out std_logic;
	 
	--FMC0 data and clock pins
   fmc0_la_i_N		  : in std_logic_vector(15 downto 0);
   fmc0_la_i_P		  : in std_logic_vector(15 downto 0);
   fmc0_la_o_N		  : out std_logic_vector(15 downto 0);
   fmc0_la_o_P		  : out std_logic_vector(15 downto 0);
	--fmc0_zdok_N		  : inout std_logic_vector(3 downto 0);	--Unused
	--fmc0_zdok_P		  : inout std_logic_vector(3 downto 0);	--Unused
   fmc0_clk0_m2c	  : in std_logic;								
	--fmc0_clk1_m2c	  : in std_logic;								   --Unused
	
	--FMC0 control pins
   fmc0_ga0	        : in std_logic;
   fmc0_ga1		     : in std_logic;
   fmc0_prsnt_n	  : in std_logic;
   fmc_i2c_scl		  : in std_logic;								  --Unused
   fmc_i2c_sda		  : in std_logic;								  --Unused

    -- GPIO, LED and CLOCK pins
   gpio             : out std_logic_vector(15 downto 0);
   led              : out std_logic_vector(7 downto 0);
   sys_clk_P		  : in std_logic;
   sys_clk_N	 	  : in std_logic
);
end gpmc_test_top;


------------------------------------------------------------------------------------
-- Architecture of rhino_proc_intrfc_top
------------------------------------------------------------------------------------

architecture rtl of gpmc_test_top is

------------------------------------------------------------------------------------
-- Declare types
------------------------------------------------------------------------------------

    type ram_type is array (63 downto 0) of std_logic_vector(15 downto 0);
	 type word32_type is array (1 downto 0) of std_logic_vector(15 downto 0);
	 type word64_type is array (3 downto 0) of std_logic_vector(15 downto 0);
	 

------------------------------------------------------------------------------------
-- Declare signals
------------------------------------------------------------------------------------

-- Define signals for the gpmc bus
    signal gpmc_clk_i_b       : std_logic;  --buffered  gpmc_clk_i
    signal gpmc_address       : std_logic_vector(25 downto 0):=(others => '0');         -- Full de-multiplexed address bus (ref. 16 bits)
    signal gpmc_data_o        : std_logic_vector(15 downto 0):="0000000000000000";      -- Register for output bus value
    signal gpmc_data_i        : std_logic_vector(15 downto 0):="0000000000000000";      -- Register for input bus value

--FMC data buffer signals
	 signal fmc0_la_o		      : std_logic_vector(15 downto 0);
	 signal fmc0_la_o_b		   : std_logic_vector(15 downto 0);
	 signal fmc0_la_i		      : std_logic_vector(15 downto 0);
	 signal fmc0_la_i_b	      : std_logic_vector(15 downto 0);
	 signal fmc0_zdok_in	      : std_logic_vector(3 downto 0);
--FMC control signsl
	 signal fmc0_we			  : std_logic;
	 signal fmc0_rd			  : std_logic;

--Clock buffer signals
	 signal fmc0_clk0_m2c_b		 : std_logic;
	 signal fmc0_clk0_m2c_slow  : std_logic;
	 signal fmc0_clk0_m2c_fb	 : std_logic;

--Other signals
    signal heartbeat			    : std_logic;
    signal dcm_locked		    : std_logic;
    signal rd_cs_en            : std_logic:='0';
    signal we_cs_en            : std_logic:='0';
	 
--Clocks
    signal sys_clk_100MHz		: std_logic;

-- Debug signals
    constant VERSION : std_logic_vector(7 downto 0) := "00000001";
	 constant ID      : std_logic_vector(7 downto 0) := "01010001";
    signal reg_bank: ram_type;
    signal led_reg : std_logic_vector(15 downto 0) := "1010101001010101";
	 signal word32_reg: word32_type := ("0101010101010101","0101010101010101");
  
-- ALIASES
    -- Support 64 memory banks, each with a maximum of 2MW 
	 ALIAS reg_bank_address: std_logic_vector(3 downto 0) IS gpmc_address(25 downto 22);  
	 -- Currently each register is 64 x 16 
	 ALIAS reg_file_address:   std_logic_vector(5 downto 0) IS gpmc_address(5 downto 0);
	 

	
	

--==========================
begin --architecture RTL
--==========================

------------------------------------------------------------------------------------
-- Instantiate input buffer for FPGA_PROC_BUS_CLK
------------------------------------------------------------------------------------

IBUFG_gpmc_clk_i : IBUFG
generic map
(
    IBUF_LOW_PWR => FALSE,
    IOSTANDARD => "DEFAULT"
)
port map
(
    I => gpmc_clk_i,
    O => gpmc_clk_i_b
);

------------------------------------------------------------------------------------
-- Instantiate diff output buffers for FMC0_LA signals
------------------------------------------------------------------------------------

generate_IBUFDS_fmc0_la_i: for i in 15 downto 0 generate

	IBUFDS_fmc0_la_i: IBUFDS
	generic map
	(
		IOSTANDARD => "LVDS_25",
		DIFF_TERM => TRUE,
		IBUF_LOW_PWR => FALSE
	)
	port map
	(
		I  => fmc0_la_i_P(i),
		IB => fmc0_la_i_N(i),
		O  => fmc0_la_i(i)
	); 

end generate generate_IBUFDS_fmc0_la_i;

------------------------------------------------------------------------------------
-- Instantiate diff output buffers for FMC0_LA signals
------------------------------------------------------------------------------------

generate_OBUFDS_fmc0_la_o: for i in 15 downto 0 generate

	OBUFDS_fmc0_la_o: OBUFDS
	generic map
	(
		IOSTANDARD => "LVDS_25"
	)
	port map
	(
		I => fmc0_la_o_b(i),
		O => fmc0_la_o_P(i),
		OB => fmc0_la_o_N(i)
	);

end generate generate_OBUFDS_fmc0_la_o;

------------------------------------------------------------------------------------
-- Instantiate clock input buffer for FMC0_CLK0_M2C
------------------------------------------------------------------------------------

IBUFG_fmc0_clk0_m2c : IBUFG
generic map
(
	IBUF_LOW_PWR => FALSE, 	-- Low power (TRUE) vs. performance (FALSE)
	IOSTANDARD => "LVCMOS33"
)
port map
(
	I => fmc0_clk0_m2c,
	O => fmc0_clk0_m2c_b
);

------------------------------------------------------------------------------------
-- Instantiate PLL for FMC0_CLK0_M2C
------------------------------------------------------------------------------------
PLL_fmc0_clk0_m2c : PLL_BASE 
generic map 
(
	BANDWIDTH          => "OPTIMIZED",
	CLKIN_PERIOD       => 3.2,					--312.5MHz, 3.2ns
	CLKOUT0_DIVIDE     => 1,
	CLKOUT1_DIVIDE     => 1,
	CLKOUT2_DIVIDE     => 1,
	CLKOUT3_DIVIDE     => 1,
	CLKOUT4_DIVIDE     => 1,
	CLKOUT5_DIVIDE     => 1,
	CLKOUT0_PHASE      => 0.000,
	CLKOUT1_PHASE      => 0.000,
	CLKOUT2_PHASE      => 0.000,
	CLKOUT3_PHASE      => 0.000,
	CLKOUT4_PHASE      => 0.000,
	CLKOUT5_PHASE      => 0.000,
	CLKOUT0_DUTY_CYCLE => 0.500,
	CLKOUT1_DUTY_CYCLE => 0.500,
	CLKOUT2_DUTY_CYCLE => 0.500,
	CLKOUT3_DUTY_CYCLE => 0.500,
	CLKOUT4_DUTY_CYCLE => 0.500,
	CLKOUT5_DUTY_CYCLE => 0.500,
	COMPENSATION       => "INTERNAL",
	DIVCLK_DIVIDE      => 4,
	CLKFBOUT_MULT      => 1,
	CLKFBOUT_PHASE     => 0.0,
	REF_JITTER         => 0.005000
)
port map
(
	CLKFBIN          => fmc0_clk0_m2c_fb,
	CLKIN            => fmc0_clk0_m2c_b,
	RST              => '0',
	CLKFBOUT         => fmc0_clk0_m2c_fb,
	CLKOUT0          => fmc0_clk0_m2c_slow,
	CLKOUT1          => open,
	CLKOUT2          => open,
	CLKOUT3          => open,
	CLKOUT4          => open,
	CLKOUT5          => open,
	LOCKED           => dcm_locked
);


------------------------------------------------------------------------------------
-- Instantiate differential input clockl buffer, for 100MHz clock (for UART)
-----------------------------------------------------------------------------------

IBUFGDS_sys_clk: IBUFGDS
generic map
(
	IOSTANDARD => "LVDS_25",
	DIFF_TERM => TRUE,
	IBUF_LOW_PWR => FALSE
)
port map
(
	I => sys_clk_P,
	IB => sys_clk_N,
	O => sys_clk_100MHz
);

------------------------------------------------------------------------------------
-- Misc signal wiring
------------------------------------------------------------------------------------

-- Map important processor bus pins to GPIO header
led <= led_reg(7 downto 0);

-- Set other outputs low
gpio    <= gpmc_clk_i_b & gpmc_n_cs(1) & gpmc_n_we & gpmc_n_oe & gpmc_a(4 downto 1) & gpmc_d(7 downto 0);
gpmc_busy_0 <= '0';
gpmc_busy_1 <= '0';

    

-----------------------------------------------------------------------------------
-- Register File: Read
------------------------------------------------------------------------------------

process (gpmc_clk_i_b,gpmc_n_cs,gpmc_n_oe,gpmc_n_we,gpmc_n_adv_ale,gpmc_d,gpmc_a)
begin
  if (gpmc_n_cs /= "1111111")  then             -- CS 1
    if gpmc_clk_i_b'event and gpmc_clk_i_b = '1' then  
		--First cycle of the bus transaction record the address
		if (gpmc_n_adv_ale = '0') then
          gpmc_address <= gpmc_a & gpmc_d;   -- Address of 16 bit word
		--Second cycle of the bus is read or write
		--Check for read
      elsif (gpmc_n_oe = '0') then
		 	case conv_integer(reg_bank_address) is
			     when 0 => gpmc_data_o <= ID & VERSION;
			     when 1 => gpmc_data_o <= led_reg;
				  when 2 => gpmc_data_o <= fmc0_la_i_b;
				  when 3 => gpmc_data_o <= reg_bank(conv_integer(reg_file_address));
			     when 4 => gpmc_data_o <= word32_reg(conv_integer(reg_file_address));
				  when others => gpmc_data_o <= (others => '0');
		   end case;
      --Check for write
	 	elsif (gpmc_n_we = '0') then
		  case conv_integer(reg_bank_address) is
			  	  when 1 => led_reg <= gpmc_data_i;
				  when 2 => fmc0_la_o <= gpmc_data_i;
				  when 3 => reg_bank(conv_integer(reg_file_address)) <= gpmc_data_i;
				  when 4 => word32_reg(conv_integer(reg_file_address)) <= gpmc_data_i;
			     when others => null;
			end case;
		end if;
     end if; 
   end if; 
end process;


-----------------------------------------------------------------------------------
-- Buffer FMC Read: Write
------------------------------------------------------------------------------------

process(sys_clk_100MHz)
begin
	if rising_edge(sys_clk_100MHz) then
			fmc0_la_o_b <= fmc0_la_o;
	end if;
end process;

-----------------------------------------------------------------------------------
-- Buffer FMC Read: Write
------------------------------------------------------------------------------------

process(sys_clk_100MHz)
begin
	if rising_edge(sys_clk_100MHz) then
			fmc0_la_i_b <= fmc0_la_i;
	end if;
end process;

------------------------------------------------------------------------------------
-- Manage the tri-state bus 
---------------------------------------------------------------------------------
gpmc_d <= gpmc_data_o when (gpmc_n_oe = '0') else (others => 'Z');
gpmc_data_i <= gpmc_d;
    
end rtl;
