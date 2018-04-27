-- fsm.vhd: Finite State Machine
-- Author(s): Martina Grzybowska (xgrzyb00)
-- kod1 = 1533492122   kod2 = 15334429104
library ieee;
use ieee.std_logic_1164.all;
-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity fsm is
port(
   CLK         : in  std_logic;
   RESET       : in  std_logic;

   -- Input signals
   KEY         : in  std_logic_vector(15 downto 0);
   CNT_OF      : in  std_logic;

   -- Output signals
   FSM_CNT_CE  : out std_logic;
   FSM_MX_MEM  : out std_logic;
   FSM_MX_LCD  : out std_logic;
   FSM_LCD_WR  : out std_logic;
   FSM_LCD_CLR : out std_logic
);
end entity fsm;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of fsm is
   type t_state is (Test1A, Test1B, Test1C, Test1D, Test1E, Test1F, Test2A, Test2B, Test2C, Test2D, Test3A, Test3B, Test3C, Test3D, Test3E, WaitingRoom, PrintFailure, PrintAllOK, Sewer, FINITO);
   signal present_state, next_state : t_state;

begin
-- -------------------------------------------------------
sync_logic : process(RESET, CLK)
begin
   if (RESET = '1') then
      present_state <= Test1A;
   elsif (CLK'event AND CLK = '1') then
      present_state <= next_state;
   end if;
end process sync_logic;

-- -------------------------------------------------------
next_state_logic : process(present_state, KEY, CNT_OF)
begin
   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when Test1A =>
      next_state <= Test1A;
      if (KEY(1) = '1') then
         next_state <= Test1B;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when Test1B =>
      next_state <= Test1B;
      if (KEY(5) = '1') then
         next_state <= Test1C;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when Test1C =>
      next_state <= Test1C;
      if (KEY(3) = '1') then
         next_state <= Test1D;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when Test1D =>
      next_state <= Test1D;
      if (KEY(3) = '1') then
         next_state <= Test1E;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -   
   when Test1E =>
      next_state <= Test1E;
      if (KEY(4) = '1') then
         next_state <= Test1F;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -   
   when Test1F =>
      next_state <= Test1F;
      if (KEY(9) = '1') then
         next_state <= Test2A;
      elsif (KEY(4) = '1') then
         next_state <= Test3A;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -   
   when Test2A =>
      next_state <= Test2A;
      if (KEY(2) = '1') then
         next_state <= Test2B;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -  
   when Test2B =>
      next_state <= Test2B;
      if (KEY(1) = '1') then
         next_state <= Test2C;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -   
   when Test2C =>
      next_state <= Test2C;
      if (KEY(2) = '1') then
         next_state <= Test2D;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when Test2D =>
      next_state <= Test2D;
      if (KEY(2) = '1') then
         next_state <= WaitingRoom;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when Test3A =>
      next_state <= Test3A;
      if (KEY(2) = '1') then
         next_state <= Test3B;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when Test3B =>
      next_state <= Test3B;
      if (KEY(9) = '1') then
         next_state <= Test3C;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when Test3C =>
      next_state <= Test3C;
      if (KEY(1) = '1') then
         next_state <= Test3D;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when Test3D =>
      next_state <= Test3D;
      if (KEY(0) = '1') then
         next_state <= Test3E;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when Test3E =>
      next_state <= Test3E;
      if (KEY(4) = '1') then
         next_state <= WaitingRoom;
      elsif (KEY(15) = '1') then
         next_state <= PrintFailure;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= Sewer;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when WaitingRoom =>
      next_state <= WaitingRoom;
      if (KEY(15) = '1') then
         next_state <= PrintAllOK;
     elsif (KEY(14 downto 0) /= "000000000000000") then
        next_state <= Sewer;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when Sewer =>
      next_state <= Sewer;
      if (KEY(15) = '1') then
         next_state <= PrintFailure;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when PrintFailure =>
      next_state <= PrintFailure;
      if (CNT_OF = '1') then
         next_state <= FINITO;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when PrintAllOK =>
      next_state <= PrintAllOK;
      if (CNT_OF = '1') then
         next_state <= FINITO;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when FINITO =>
      next_state <= FINITO;
      if (KEY(15) = '1') then
        next_state <= Test1A;
      end if;
  -- - - - - - - - - - - - - - - - - - - - - - -
  when others =>
      next_state <= Test1A;
  end case;
end process next_state_logic;

-- -------------------------------------------------------
output_logic : process(present_state, KEY)
begin
   FSM_CNT_CE     <= '0';
   FSM_MX_MEM     <= '0';
   FSM_MX_LCD     <= '0';
   FSM_LCD_WR     <= '0';
   FSM_LCD_CLR    <= '0';

   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PrintFailure =>
      FSM_CNT_CE     <= '1';
      FSM_MX_LCD     <= '1';
      FSM_LCD_WR     <= '1';
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PrintAllOK =>
      FSM_CNT_CE     <= '1';
      FSM_MX_LCD     <= '1';
      FSM_LCD_WR     <= '1';
      FSM_MX_MEM     <= '1';
   -- - - - - - - - - - - - - - - - - - - - - - -
   when FINITO =>
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
      if (KEY(14 downto 0) /= "000000000000000") then
          FSM_LCD_WR     <= '1';
      end if;
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
      end if;
   end case;
end process output_logic;

end architecture behavioral;

