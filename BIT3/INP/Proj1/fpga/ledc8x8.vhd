----------- INP PROJ1 --------------
-- Martina Grzybowska -- xgrzyb00 --
-------------- 2016 ----------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity ledc8x8 is
port ( 
		ROW		: out std_logic_vector (0 to 7);
		LED		: out std_logic_vector (0 to 7);
		RESET	: in std_logic;
        SMCLK   : in std_logic
);
end ledc8x8;

architecture main of ledc8x8 is

    signal ledz: std_logic_vector(7 downto 0);
    signal rowz: std_logic_vector(7 downto 0);
    signal ce_cnt: std_logic_vector(7 downto 0);
    signal switcher: std_logic_vector(22 downto 0);
    signal ce: std_logic;
    signal initials_MG: std_logic;

begin
	generating_ce: process(SMCLK, RESET)
    begin
		if RESET = '1' then
			ce_cnt <= "00000000";
			switcher <= "00000000000000000000000";
        elsif SMCLK'event and SMCLK = '1' then
            ce_cnt <= ce_cnt + 1;
            switcher <= switcher + 1;
            if switcher = "11100001000000000000000" then
                initials_MG <= not initials_MG; 
                switcher <= "00000000000000000000000";   
         end if; 
        end if;
    end process generating_ce;
    
    ce <= '1' when ce_cnt = X"FF" else '0';  
    
    rotate: process(RESET, ce, SMCLK)
	begin	
		if RESET = '1' then
			rowz <= "10000000"; 
		elsif SMCLK'event and SMCLK = '1' and ce = '1' then
			rowz <= rowz(0) & rowz(7 downto 1);
		end if;
    end process rotate;

    decoder: process(rowz, initials_MG)
	begin
    
        if initials_MG = '1' then
            case rowz is 
				when "10000000" => ledz <= "01111101";
				when "01000000" => ledz <= "00111001";
				when "00100000" => ledz <= "01010101";
				when "00010000" => ledz <= "01101101";
				when "00001000" => ledz <= "01111101";
				when "00000100" => ledz <= "01111101";
				when "00000010" => ledz <= "11111111";
                when "00000001" => ledz <= "11111111";
                when others =>  ledz <= "11111111";
            end case;
        
        else 
            case rowz is 
				when "10000000" => ledz <= "11000011";
				when "01000000" => ledz <= "10111101";
				when "00100000" => ledz <= "01111111";
				when "00010000" => ledz <= "01100001";
				when "00001000" => ledz <= "10111101";
				when "00000100" => ledz <= "11000011";
				when "00000010" => ledz <= "11111111";
                when "00000001" => ledz <= "11111111";
                when others => ledz <= "11111111";
            end case;
        end if;
    end process decoder;
    
    ROW <= rowz;
    LED <= ledz; 

end main;
