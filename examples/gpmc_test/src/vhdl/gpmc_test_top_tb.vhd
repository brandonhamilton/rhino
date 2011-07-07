--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   10:28:18 06/24/2011
-- Design Name:   
-- Module Name:   E:/dropbox/My Dropbox/work/vermeer/svn/DHRA0140/ConceptExploration/prototype/gateware/gpmc_test/gpmc_test_top_tb.vhd
-- Project Name:  gpmc_test
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: gpmc_test_top
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY gpmc_test_top_tb IS
END gpmc_test_top_tb;
 
ARCHITECTURE behavior OF gpmc_test_top_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT gpmc_test_top
    PORT(
         gpmc_a : IN  std_logic_vector(10 downto 1);
         gpmc_d : INOUT  std_logic_vector(15 downto 0);
         gpmc_clk_i : IN  std_logic;
         gpmc_n_cs : IN  std_logic_vector(6 downto 0);
         gpmc_n_we : IN  std_logic;
         gpmc_n_oe : IN  std_logic;
         gpmc_n_adv_ale : IN  std_logic;
         gpmc_n_wp : IN  std_logic;
         gpmc_busy_0 : OUT  std_logic;
         gpmc_busy_1 : OUT  std_logic;
         fmc0_la_i_N : IN  std_logic_vector(15 downto 0);
         fmc0_la_i_P : IN  std_logic_vector(15 downto 0);
         fmc0_la_o_N : OUT  std_logic_vector(15 downto 0);
         fmc0_la_o_P : OUT  std_logic_vector(15 downto 0);
         fmc0_clk0_m2c : IN  std_logic;
         fmc0_ga0 : IN  std_logic;
         fmc0_ga1 : IN  std_logic;
         fmc0_prsnt_n : IN  std_logic;
         fmc_i2c_scl : IN  std_logic;
         fmc_i2c_sda : IN  std_logic;
         gpio : OUT  std_logic_vector(15 downto 0);
         led : OUT  std_logic_vector(7 downto 0);
         sys_clk_P : IN  std_logic;
         sys_clk_N : IN  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal gpmc_a : std_logic_vector(10 downto 1) := (others => '0');
   signal gpmc_clk : std_logic := '0';
   signal gpmc_n_cs : std_logic_vector(6 downto 0) := (others => '0');
   signal gpmc_n_we : std_logic := '0';
   signal gpmc_n_oe : std_logic := '0';
   signal gpmc_n_adv_ale : std_logic := '0';
   signal gpmc_n_wp : std_logic := '0';
   signal fmc0_la_i_N : std_logic_vector(15 downto 0) := (others => '0');
   signal fmc0_la_i_P : std_logic_vector(15 downto 0) := (others => '0');
   signal fmc0_clk0_m2c : std_logic := '0';
   signal fmc0_ga0 : std_logic := '0';
   signal fmc0_ga1 : std_logic := '0';
   signal fmc0_prsnt_n : std_logic := '0';
   signal fmc_i2c_scl : std_logic := '0';
   signal fmc_i2c_sda : std_logic := '0';
   signal sys_clk_P : std_logic := '0';
   signal sys_clk_N : std_logic := '0';

	--BiDirs
   signal gpmc_d : std_logic_vector(15 downto 0);

 	--Outputs
   signal gpmc_busy_0 : std_logic;
   signal gpmc_busy_1 : std_logic;
   signal fmc0_la_o_N : std_logic_vector(15 downto 0);
   signal fmc0_la_o_P : std_logic_vector(15 downto 0);
   signal gpio : std_logic_vector(15 downto 0);
   signal led : std_logic_vector(7 downto 0);
   -- No clocks detected in port list. Replace <clock> below with 
   -- appropriate port name 
 
   signal   gpmc_fclk: std_logic:='0';  -- internal bus clock
   constant gpmc_fclk_period: time := 12 ns;
	signal   sys_clk: std_logic:='0'; -- internal system clock for FPGA
	constant sys_clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: gpmc_test_top PORT MAP (
          gpmc_a => gpmc_a,
          gpmc_d => gpmc_d,
          gpmc_clk_i => gpmc_clk,
          gpmc_n_cs => gpmc_n_cs,
          gpmc_n_we => gpmc_n_we,
          gpmc_n_oe => gpmc_n_oe,
          gpmc_n_adv_ale => gpmc_n_adv_ale,
          gpmc_n_wp => gpmc_n_wp,
          gpmc_busy_0 => gpmc_busy_0,
          gpmc_busy_1 => gpmc_busy_1,
          fmc0_la_i_N => fmc0_la_i_N,
          fmc0_la_i_P => fmc0_la_i_P,
          fmc0_la_o_N => fmc0_la_o_N,
          fmc0_la_o_P => fmc0_la_o_P,
          fmc0_clk0_m2c => fmc0_clk0_m2c,
          fmc0_ga0 => fmc0_ga0,
          fmc0_ga1 => fmc0_ga1,
          fmc0_prsnt_n => fmc0_prsnt_n,
          fmc_i2c_scl => fmc_i2c_scl,
          fmc_i2c_sda => fmc_i2c_sda,
          gpio => gpio,
          led => led,
          sys_clk_P => sys_clk_P,
          sys_clk_N => sys_clk_N
        );

    -- Clock process definitions FCLK
   gpmc_fclk_process :process
   begin
		gpmc_fclk <= '0';
		wait for gpmc_fclk_period/2;
		gpmc_fclk <= '1';
		wait for gpmc_fclk_period/2;
   end process;
	
	   -- Clock process definitions FCLK
   sys_clk_process :process
   begin
		sys_clk <= '0';
		wait for sys_clk_period/2;
		sys_clk <= '1';
		wait for sys_clk_period/2;
   end process;
	
	-- Generate dff clock
		sys_clk_P <= sys_clk;
		sys_clk_N <= not(sys_clk);
	

 
   -- Stimulus process
   stim_proc: process
   begin
	
	   -- iniit
      gpmc_n_cs <= "1111111";
      gpmc_n_oe <= '1';
      gpmc_n_we <= '1';
		gpmc_clk  <= '0';
		gpmc_n_adv_ale <= '1';
		gpmc_a <= (others => '1');
		gpmc_d <= "ZZZZZZZZZZZZZZZZ";
      wait for (gpmc_fclk_period/2)*11;
		
		-- time 0
		gpmc_clk  <= '0';
		gpmc_n_cs <= "1111110";
		gpmc_n_we <= '1';
		gpmc_n_oe <= '1';
		gpmc_n_adv_ale <= '1';
      gpmc_a <= "0000010000";
		gpmc_d <= "0000000000000000";
		wait for gpmc_fclk_period;
		
		-- time 1
		gpmc_clk  <= '0';
		gpmc_n_cs <= "1111110";
		gpmc_n_we <= '1';
		gpmc_n_oe <= '1';
		gpmc_n_adv_ale <= '0';
      gpmc_a <= "0000010000";
		gpmc_d <= "0000000000000000";
		wait for gpmc_fclk_period;
		
		-- time 2
		gpmc_clk  <= '1';
		gpmc_n_cs <="1111110";
		gpmc_n_we <= '1';
		gpmc_n_oe <= '1';
		gpmc_n_adv_ale <= '0';
      gpmc_a <= "0000100000";
		gpmc_d <= "0000000000000000";
		wait for gpmc_fclk_period;
		

		-- time 3
		gpmc_clk  <= '0';
		gpmc_n_cs <= "1111110";
		gpmc_n_we <= '0';
		gpmc_n_oe <= '1';
		gpmc_n_adv_ale <= '1';
		gpmc_a <= "0000100000";
		gpmc_d <= "0000000011111111";
		wait for gpmc_fclk_period;
		
		-- time 4
		gpmc_clk  <= '1';
		gpmc_n_cs <= "1111110";
		gpmc_n_we <= '0';
		gpmc_n_oe <= '1';
		gpmc_n_adv_ale <= '1';
		gpmc_a <= (others => '0');
		gpmc_d <= "0000000010101111";
		wait for gpmc_fclk_period;
		
		-- time 5
		gpmc_clk  <= '0';
		gpmc_n_cs <=  "1111111";
		gpmc_n_we <= '1';
		gpmc_n_oe <= '1';
		gpmc_n_adv_ale <= '1';
		gpmc_a <= (others => '0');
		gpmc_d <= (others => 'Z');
		wait for gpmc_fclk_period;
		
		-- time 6
		gpmc_clk  <= '1';
		gpmc_n_cs <=  "1111111";
		gpmc_n_we <= '1';
		gpmc_n_oe <= '1';
		gpmc_n_adv_ale <= '1';
		gpmc_a <= (others => '0');
		gpmc_d <= (others => 'Z');
		wait for gpmc_fclk_period;
		
		
		gpmc_clk <= '0';
		wait for gpmc_fclk_period*10;
		
		
		
      -- insert stimulus here 

      wait;
   end process;

END;
