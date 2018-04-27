-- cpu.vhd: Simple 8-bit CPU (BrainLove interpreter)
-- Copyright (C) 2016 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Martina Grzybowska, xgrzyb00
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti
   
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (1) / zapis (0)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

 -- zde dopiste potrebne deklarace signalu;
signal PC_reg_inc : std_logic;
signal PC_reg_dec : std_logic;
signal PC_reg_address : std_logic_vector (11 downto 0);

signal CNT_reg_inc : std_logic;
signal CNT_reg_dec : std_logic;
signal CNT_reg_set: std_logic;
signal CNT_reg_cnt : std_logic_vector (7 downto 0);

signal TMP_reg_val : std_logic_vector (7 downto 0);
signal Tmp_reg_load : std_logic; 
signal TMP_reg_address : std_logic_vector (7 downto 0);

signal PTR_reg_inc : std_logic;
signal PTR_reg_dec : std_logic;
signal PTR_reg_address : std_logic_vector (9 downto 0);

type INS_type is 
(dec_ptr, inc_ptr, dec_val, 
 inc_val, read_act, print_act, 
 act_to_tmp, tmp_to_act, halt, 
 while_0, while_0e, skip);

signal INS_reg : INS_type;


type FSM_state is 
(S_dec_pc, S_inc_pc, S_dec_ptr0, 
 S_inc_ptr0, S_dec_ptr1, S_inc_ptr1, 
 S_read_act0, S_read_act1, S_print_act0, 
 S_print_act1, S_act_to_tmp0, S_act_to_tmp1, S_tmp_to_act, 
 S_while0, S_while0e, S_while1, S_whileBLA, S_whileBLAe,
 S_while1e, S_while2, S_while2e, S_while3, 
 S_while3e, S_while4e, S_skip, 
 S_idle, S_fetch, S_decode, S_halt);

signal P_state : FSM_state;
signal N_state : FSM_state;

signal sig : std_logic_vector(1 downto 0);

begin

 -- zde dopiste vlastni VHDL kod
-----------------------
----- REGISTER PC -----
PC_process: process (RESET, CLK, PC_reg_dec, PC_reg_inc)
begin
   if (RESET = '1') then PC_reg_address <= (others => '0');
   elsif (CLK'event) and (CLK = '1') and (RESET = '0') then
      if (PC_reg_dec = '1') then PC_reg_address <= PC_reg_address - 1;
      elsif (PC_reg_inc = '1') then PC_reg_address <= PC_reg_address + 1;
      end if;
   end if;
end process;
CODE_ADDR <= PC_reg_address;
-------------------------------   
----- COUNTER ---------- 
CNT_process: process(RESET, CLK, CNT_reg_inc, CNT_reg_dec, CNT_reg_set)
begin
   if (RESET = '1') then CNT_reg_cnt <= (others => '0');
   elsif (CLK'event) and (CLK = '1') and (RESET = '0') then
      if (CNT_reg_dec = '1') then CNT_reg_cnt <= CNT_reg_cnt - 1;
      elsif (CNT_reg_inc = '1') then CNT_reg_cnt <= CNT_reg_cnt + 1;
      end if;
   end if;
   if (CNT_reg_set = '1') then CNT_reg_cnt <= X"01";
   end if;
end process;

-------------------------------   
----- MEMORY POINTER ----------  
PTR_process: process(RESET, CLK, PTR_reg_dec, PTR_reg_inc)
begin
   if (RESET = '1') then PTR_reg_address <= (others => '0');
   elsif (CLK'event) and (CLK = '1') and (RESET = '0') then
      if (PTR_reg_dec = '1') then PTR_reg_address <= PTR_reg_address - 1;
      elsif (PTR_reg_inc = '1') then PTR_reg_address <= PTR_reg_address + 1;
      end if;
   end if;
end process;
DATA_ADDR <= PTR_reg_address;
-------------------------------   
------------ TMP --------------
TMP_process: process(RESET, CLK, TMP_reg_val, Tmp_reg_load)
begin
   if (RESET = '1') then TMP_reg_address <= (others => '0');
   elsif (CLK'event) and (CLK = '1') and (RESET = '0') then
      if (Tmp_reg_load = '1') then
         TMP_reg_val <= DATA_RDATA;
      end if;
   end if;
   TMP_reg_address <= TMP_reg_val;
end process;

---------------------------------------------------   
----- FINITE STATE MACHINE PRESENT STATE ----------
FSM_p_state : process(EN, RESET, CLK)
begin
   if (RESET = '1') then
      P_state <= S_idle;
   elsif (CLK'event) and (CLK = '1') and (RESET = '0') then
      if (EN = '1') then
         P_state <= N_state;
      end if;
   end if;
end process;
-------------------------------   
----- INSTRUCTION DECODER -----    
INS_decoder: process(CODE_DATA)
begin
   case (CODE_DATA) is
      when X"3C" => INS_reg <= dec_ptr; 
      when X"3E" => INS_reg <= inc_ptr;
      when X"2D" => INS_reg <= dec_val;
      when X"2B" => INS_reg <= inc_val;
      when X"5D" => INS_reg <= while_0e; --]
      when X"5B" => INS_reg <= while_0; --[
      when X"2C" => INS_reg <= read_act;
      when X"2E" => INS_reg <= print_act;
      when X"21" => INS_reg <= tmp_to_act;
      when X"24" => INS_reg <= act_to_tmp; -- $
      when X"00" => INS_reg <= halt;
      when others => INS_reg <= skip; 
   end case ;
end process;
----------------------------  
----- MULTIPLEXOR ----------
MPX_process : process (CLK, IN_DATA, DATA_RDATA, TMP_reg_address, sig) 
begin
   case sig is
      when "00" => DATA_WDATA <= IN_DATA;
      when "01" => DATA_WDATA <= DATA_RDATA - 1;
      when "10" => DATA_WDATA <= DATA_RDATA + 1;
      when "11" => DATA_WDATA <= TMP_reg_address;
      when others => 
   end case;
end process;
----------------------------  
OUT_DATA <= DATA_RDATA;
----------------------------------------------------------------------------------------------------------------   
----- FINITE STATE MACHINE ----------
FSM_process : process(P_state, INS_reg, OUT_BUSY, IN_VLD, DATA_RDATA, CNT_reg_cnt, CODE_DATA, sig)

begin  
   CODE_EN <= '0';
   DATA_EN <= '0';
   DATA_RDWR <= '0';
   OUT_WE <= '0';
   IN_REQ <= '0';
   
   PC_reg_inc <= '0';
   PC_reg_dec <= '0';

   CNT_reg_inc <= '0';
   CNT_reg_dec <= '0';
   CNT_reg_set <= '0';

   PTR_reg_inc <= '0';
   PTR_reg_dec <= '0';

   Tmp_reg_load <= '0';

   sig <= "00";  

-------------------------------------  

   case P_state is
      
      when S_idle => N_state <= S_fetch;
      -------------------------------------
      when S_fetch => N_state <= S_decode;
         CODE_EN <= '1';
      ------------------------------------- 
      when S_decode =>
         case (INS_reg) is
            when dec_ptr => N_state <= S_dec_pc;
            when inc_ptr => N_state <= S_inc_pc;
            when dec_val => N_state <= S_dec_ptr0;
            when inc_val => N_state <= S_inc_ptr0;
            when read_act => N_state <= S_read_act0;
            when print_act => N_state <= S_print_act0;
            when tmp_to_act => N_state <= S_tmp_to_act;
            when act_to_tmp => N_state <= S_act_to_tmp0;
            when while_0 => N_state <= S_while0;
            when while_0e => N_state <= S_while0e;
            when skip => N_state <= S_skip;
            when halt => N_state <= S_halt;
            when others => N_state <= S_fetch;
         end case ;
      ------------------------------------- 
      when S_halt => N_state <= S_halt;
      -------------------------------------
      when S_skip => N_state <= S_fetch;
         PC_reg_inc <= '1';
         PC_reg_dec <= '0';

      -------------------------------------
      --------------  PC ------------------
      when S_inc_pc => N_state <= S_fetch;
         PC_reg_inc <= '1';
         PTR_reg_inc <= '1';
      -------------------------------------
      when S_dec_pc => N_state <= S_fetch;
         PC_reg_inc <= '1';
         PTR_reg_dec <= '1';

      -------------------------------------
      --------------  PTR -----------------
      when S_inc_ptr0 => N_state <= S_inc_ptr1;
         DATA_RDWR <= '1';
         DATA_EN <= '1';

      -------------------------------------
      when S_inc_ptr1 => N_state <= S_fetch;
         DATA_RDWR <= '0';
         DATA_EN <= '1';         
         PC_reg_inc <= '1';
         sig <= "10"; -- when "10" => DATA_WDATA <= DATA_RDATA + 1;
      -------------------------------------
      when S_dec_ptr0 => N_state <= S_dec_ptr1;
         DATA_RDWR <= '1';
         DATA_EN <= '1';
      -------------------------------------
      when S_dec_ptr1 => N_state <= S_fetch;
         DATA_RDWR <= '0';
         DATA_EN <= '1';
         PC_reg_inc <= '1';
         sig <= "01"; --"01" => DATA_WDATA <= DATA_RDATA - 1;
         
      ---------------------------------------
      --------------  WRITE -----------------
      when S_print_act0 => 
         if (OUT_BUSY = '0') then 
            N_state <= S_print_act1;
            DATA_EN <= '1';
            DATA_RDWR <= '1';
         else 
            N_state <= S_print_act0;
         end if;
      -------------------------------------
      when S_print_act1 => N_state <= S_fetch;
         OUT_WE <= '1';
         PC_reg_inc <= '1';

      ---------------------------------------
      --------------  READ ------------------
      when S_read_act0 => 
         IN_REQ <= '1';
         if (IN_VLD = '1') then N_state <= S_read_act1;
         else N_state <= S_read_act0;
         end if;
      ---------------------------------------
      when S_read_act1 => N_state <= S_fetch;
         DATA_RDWR <= '0';
         DATA_EN <= '1';
         sig <= "00"; --"00" => DATA_WDATA <= IN_DATA;
         PC_reg_inc <= '1';

      -------------------------------------
      -------------- TMP ------------------
      when S_act_to_tmp0 => N_state <= S_act_to_tmp1;
         DATA_EN <= '1';
         DATA_RDWR <= '1';
      -------------------------------------
      when S_act_to_tmp1 => N_state <= S_fetch;
         PC_reg_inc <= '1';
         Tmp_reg_load <= '1';
      -------------------------------------
      when S_tmp_to_act => N_state <= S_fetch;
         DATA_EN <= '1';
         sig <= "11"; --"11" => DATA_WDATA <= TMP_reg_address;
         PC_reg_inc <= '1';
      -------------------------------------
      


      --------------- WHILE ---------------
      when S_while0 => N_state <= S_while1;
         PC_reg_inc <= '1';
         DATA_RDWR <= '1';
         DATA_EN <= '1';
      -------------------------------------
      when S_while1 => 
         if (DATA_RDATA = X"00") then 
            N_state <= S_whileBLA;
            CNT_reg_set <= '1';
         else N_state <= S_fetch;
         end if;
      -------------------------------------
      when S_whileBLA => N_state <= S_while2;
         CODE_EN <= '1';
      -------------------------------------
      when S_while2 => 
         if (CNT_reg_cnt = X"00") then N_state <= S_fetch;
         else 
            N_state <= S_while3;
         end if;
      -------------------------------------
      when S_while3 => N_state <= S_whileBLA;
         if (CODE_DATA = X"5B") then CNT_reg_inc <= '1'; -- [ B
         elsif (CODE_DATA = X"5D") then CNT_reg_dec <= '1'; -- ] D
         end if;
         PC_reg_inc <= '1';

      ---------------- END ---------------------
      when S_while0e => N_state <= S_while1e;
         DATA_EN <= '1';
         DATA_RDWR <= '1';
      -------------------------------------
      when S_while1e =>
         if (DATA_RDATA = X"00") then 
            N_state <= S_fetch;
            PC_reg_inc <= '1';
         else N_state <= S_whileBLAe;
            CNT_reg_set <= '1';
            PC_reg_dec <= '1';
         end if;
      -------------------------------------
      when S_whileBLAe => N_state <= S_while2e;
         CODE_EN <= '1';
      -------------------------------------       
      when S_while2e =>
            if (CNT_reg_cnt = X"00") then N_state <= S_fetch;
            else 
               N_state <= S_while3e;
            end if;
      -------------------------------------
      when S_while3e => N_state <= S_while4e;
         if (CODE_DATA = X"5D") then CNT_reg_inc <= '1'; -- [ B
         elsif (CODE_DATA = X"5B") then CNT_reg_dec <= '1'; -- ] D
         end if;
      -------------------------------------
      when S_while4e => N_state <= S_whileBLAe;
         if (CNT_reg_cnt = X"00") then PC_reg_inc <= '1';
         else PC_reg_dec <= '1';
         end if;
      -------------------------------------    
      when others => N_state <= S_fetch;
   end case;
   -------------------------------------
end process;
-------------------------------------
end behavioral;
