<?php
/**
 *	Filename: parser.php
 *	Date: 26.2.2017
 *	Author: Martina Grzybowska, xgrzyb00@stud.fit.vutbr.cz
 * 	Project: IPP 16/17, FSM determinization
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	/**
	* parsingAscent reads input contents char after char 
	*/
	function parsingAscent(&$inputFSM, &$origStates, &$origSymbols, &$origRules, &$startState, &$origFinalStates) {
		
		$INPUT_CORRECT = FALSE;
		
		tokenCombo($token, $inputFSM);
		if ($token === LEFT_ROUND) { // (
			tokenCombo($token, $inputFSM);
			if ($token === LEFT_CURLY) { // ({
				tokenCombo($token, $inputFSM);
				if ($token !== RIGHT_CURLY) {
					parseStates($token, $inputFSM, $origStates); //({aa}
				}
				tokenCombo($token, $inputFSM);
				if ($token === COMMA) { //({aa},
					tokenCombo($token, $inputFSM);
					if ($token === LEFT_CURLY) { //({aa},{
						tokenCombo($token, $inputFSM);
						if ($token === RIGHT_CURLY) {
							errorExit(SEM_CODE, "SEM ERR - Input alphabet is empty\n");
						}
						parseSymbols($token, $inputFSM, $origSymbols); //({aa},{'a'}
						tokenCombo($token, $inputFSM);
						if ($token === COMMA) { //({aa},{'a'},
							tokenCombo($token, $inputFSM);
							if ($token === LEFT_CURLY) { //({aa},{'a'},{
								tokenCombo($token, $inputFSM);
								if ($token !== RIGHT_CURLY) {
									parseRules($token, $inputFSM, $origStates, $origSymbols, $origRules, $index); //({aa},{'a'},{aa'a'->aa}
								}
								tokenCombo($token, $inputFSM);
								if ($token === COMMA) { //({aa},{'a'},{aa'a'->aa},
									tokenCombo($token, $inputFSM);
									if ($token !== COMMA) {
										parseInitialState($token, $inputFSM, $origStates, $startState);
									}
									else {
										errorExit(LEX_SYN_CODE, "SYN ERR - no initial state declared\n");
									}
									cutSpaces($token, $inputFSM);
									if ($token === COMMA) {
										tokenCombo($token, $inputFSM);
										if ($token === LEFT_CURLY) {
											tokenCombo($token, $inputFSM);
											if ($token !== RIGHT_CURLY) {
												parseFinalStates($token, $inputFSM, $origStates, $origFinalStates);
											}
											tokenCombo($token, $inputFSM);
											if ($token === RIGHT_ROUND) {
												//if there is some garbage after the closing RIGHT_ROUND
												if (strlen($inputFSM) !== 0) {
													tokenCombo($token, $inputFSM);
													//if the garbage is not a white char, kill it with fire
													if (!ctype_space($token)) {
														errorExit(LEX_SYN_CODE, SYN_QUOTE);
													}
												}
												sort($origRules);
												sort($origStates);
												sort($origFinalStates);
												$INPUT_CORRECT = TRUE;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		if ($INPUT_CORRECT == FALSE) {
			errorExit(LEX_SYN_CODE, SYN_QUOTE);
		}
	}
?>