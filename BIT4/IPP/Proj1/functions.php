<?php
/**
 *	Filename: functions.php
 *	Date: 26.2.2017
 *	Author: Martina Grzybowska, xgrzyb00@stud.fit.vutbr.cz
 * 	Project: IPP 16/17, FSM determinization
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	/**
	* getToken reads input string char after char
	*/
	function getToken(&$token, &$inputFSM) {
		$token = mb_substr($inputFSM, 0, 1);
		$inputFSM = mb_substr($inputFSM, 1);
	}
	/**
	* errorExit exits the program immediately
	*/
	function errorExit($errCode, $errQuote) {
		fprintf(STDERR, $errQuote);
		exit($errCode);
	}
	/**
	* cutSpaces rips out all white chars and comments
	*/
	function cutSpaces(&$token, &$inputFSM) {
		while (($token === "#") or ctype_space($token)) {
			if ($token === "#") {
				while ($token !== "\n") {
					//if we reach the end of file/string
					if (strlen($inputFSM) == 0) {
						$token = ' ';
						break;
					}
					getToken($token, $inputFSM);
				}
			}
			else {
				if (strlen($inputFSM) == 0) {
						$token = ' ';
						break;
				}
				getToken($token, $inputFSM);
			}
		}
	}
	/**
	* tokenCombo is a black magic function 
	*/
	function tokenCombo(&$token, &$inputFSM) {
		getToken($token, $inputFSM);
		cutSpaces($token, $inputFSM);
	}
	/**
	* rollingDung takes a sequence of tokens and merges them
	*/
	function rollingDung(&$token, &$inputFSM) {
		$rollingDung = "";
		//if the first token is alphabetic, roll the tokens till they are either alpha, digit or underscore
		if (ctype_alpha($token)) {
			while (ctype_alnum($token) or ($token === UNDERSCORE)) {
				$rollingDung .= $token;
				getToken($token, $inputFSM);
			}
		}
		else {
			errorExit(LEX_SYN_CODE, LEX_QUOTE);
		}
		// state name cannot end with an underscore
		if (mb_substr($rollingDung, mb_strlen($rollingDung) - 1) === UNDERSCORE) {
			errorExit(LEX_SYN_CODE, LEX_QUOTE);
		}

		return $rollingDung;
	}
	/**
	* printHelp echos a script description to STDIN if parameter --help is set
	*/
	function printHelp() {
		echo "A script for processing and determinizating a finite automaton text representation\n";
		echo "It allows for these arguments:\n";
		echo "--help: prints a short hint message\n";
		echo "--input=filename: input file in UTF-8 encoding contaning a text representation of a finite automaton to be analyzed\n";
		echo "--output=filename: output file in UTF-8 encoding contaning a text representation of a finite automaton in a desired output form\n";
		echo "-e, --no-epsilon-rules: used for simple elimination of epsilon rules, cannot be combined with -d, --determinization\n";
		echo "-d, --determinization: executes a process of determinization without inaccessible states, cannot be combined with -e, --no-epsilon-rules\n";
		echo "-i, --case-insensitive: case is not taken into consideration when comparing states and symbols\n";
		echo "If neither -e nor -d were set, the input is simply validated and printed in a normalized form\n";
	}
	/**
	* printFSM prints a normalized output of FSM
	*/
	function printFSM (&$origStates, &$origSymbols, &$origRules, &$startState, &$origFinalStates, &$outputFile) {
		
		$outputText = LEFT_ROUND . "\n";
		$outputText .= LEFT_CURLY . implode(", ", $origStates) . RIGHT_CURLY . ",\n";
		$outputText .= LEFT_CURLY . "'" . implode("', '", $origSymbols) . "'" . RIGHT_CURLY . ",\n" . LEFT_CURLY . "\n";
		
		$counter = count($origRules);
		for ($i = 0; $i < $counter; $i++) {
			$glue = ",\n";
			if ($i == ($counter - 1)) {
				$glue = "\n";
			}
			$outputText .= $origRules[$i]->initState . " '" . $origRules[$i]->inputSymbol . "' " . "-> " . $origRules[$i]->targetState . $glue;
		}
		$outputText .= RIGHT_CURLY . ",\n" . $startState . ",\n" . LEFT_CURLY . implode(", ", $origFinalStates) . RIGHT_CURLY . "\n" . RIGHT_ROUND . "\n";

		if ($outputFile === '') {
			echo $outputText;
		}
		else {
			fwrite($outputFile, $outputText);
			fclose($outputFile);
		}
	}
	/**
	* parseFinalStates reads the final states from the respective set and checks whether they are defined
	*/
	function parseFinalStates(&$token, &$inputFSM, &$origStates, &$origFinalStates) {
		
		// rolls the dung till it reaches another state or the end of states set
		$rollingDung = rollingDung($token, $inputFSM);

		// check whether the state is defined
		if (in_array($rollingDung, $origStates)) {
			if (!in_array($rollingDung, $origFinalStates)) {
				array_push($origFinalStates, $rollingDung);
			}
		}
		else {
			errorExit(SEM_CODE, "SEM ERR - final state not included in states array\n");
		}
		
		cutSpaces($token, $inputFSM);
		
		// if next token is COMMA, parse again
		if ($token === COMMA) {
			tokenCombo($token, $inputFSM);
			parseFinalStates($token, $inputFSM, $origStates, $origFinalStates);
			return;
		}
		// if it is RIGHT CURLY, finish pasing of final states
		elseif ($token === RIGHT_CURLY) {
			$origFinalStates = array_unique($origFinalStates); //delete all duplicates
			sort($origFinalStates); //sort the final states alphabetically
			return;
		}
		else {
			errorExit(LEX_SYN_CODE, SYN_QUOTE);
		}
	}
	/**
	* parseInitialState reads the  init state and checks whether it is defined
	*/
	function parseInitialState(&$token, &$inputFSM, &$origStates, &$start_state) {
		
		$rollingDung = rollingDung($token, $inputFSM);

		// check whether the state is defined
		if (in_array($rollingDung, $origStates)){
			$start_state = $rollingDung;
			return;
		}
		else {
			errorExit(SEM_CODE, "SEM ERR - starting state not included in states array\n");
		}
	}
	/**
	* parseRules reads input rules, checks whether the respective states and symbols are defined
	* and if every condition is met, it creates an object of the rule and pushes it into the rules array 
	*/
	function parseRules(&$token, &$inputFSM, &$origStates, &$origSymbols, &$origRules, &$index) {

		$rollingDung = rollingDung($token, $inputFSM);

		cutSpaces($token, $inputFSM);
		if ($token !== APOSTROPHE) {
			errorExit(LEX_SYN_CODE, LEX_QUOTE);
		}
		// create a new object of the rule
		$single_rule = new InputRules();
		
		// check whether the state is defined
		if (in_array($rollingDung, $origStates)) {
			$single_rule->initState = $rollingDung;
		}
		else {
			errorExit(SEM_CODE, "SEM ERR - initial state in a rule not included in states array\n");
		}

		getToken($token, $inputFSM);
		
		if ($token === APOSTROPHE) { // if we have ''
			getToken($token, $inputFSM); 
			if ($token === APOSTROPHE) { // if we have '''
				getToken($token, $inputFSM);
				if ($token == APOSTROPHE) { // ''''
					if (in_array("''", $origSymbols)) {
						$single_rule->inputSymbol = "''";
						getToken($token, $inputFSM);
					}
					else {
						errorExit(SEM_CODE, "SEM ERR - input symbol not included in symbols array\n");
					}
				}
				else {
					errorExit(LEX_SYN_CODE, SYN_QUOTE);
				}
			}
			else { // if we do not have ''', call it is eps
				$single_rule->inputSymbol = '';
			}
		}
		else { //if we do not, parse normal symbol
			if (in_array($token, $origSymbols)) {
				$single_rule->inputSymbol = $token;
				getToken($token, $inputFSM);
				if ($token !== APOSTROPHE) {
					errorExit(LEX_SYN_CODE, SYN_QUOTE);
				}
				getToken($token, $inputFSM);
			}
			else {
				errorExit(SEM_CODE, "SEM ERR - input symbol not included in symbols array\n");
			}
		}
		
		cutSpaces($token, $inputFSM);
		if ($token === DASH) {
			getToken($token, $inputFSM);
			if ($token === RIGHT_ARROW) {
				tokenCombo($token, $inputFSM);
				$rollingDung = rollingDung($token, $inputFSM);
				if (in_array($rollingDung, $origStates)) {
					$single_rule->targetState = $rollingDung;
					$isThere = FALSE;
					foreach ($origRules as $key) {
						if ($key->initState === $single_rule->initState) {
							if ($key->inputSymbol === $single_rule->inputSymbol) {
								if ($key->targetState === $single_rule->targetState) {
									$isThere = TRUE;
								}
							}
						}
					}
					if ($isThere === FALSE) { //if the rule does not exist yet, push it
						array_push($origRules, $single_rule);
					}
				}
				else {
					errorExit(SEM_CODE, "SEM ERR - target state not included in states array\n");
				}
				cutSpaces($token, $inputFSM);
				if ($token === COMMA) { 
					tokenCombo($token, $inputFSM);
					parseRules($token, $inputFSM, $origStates, $origSymbols, $origRules, $index);
					return;
				}
				elseif ($token === RIGHT_CURLY) { //finish if there are no other rules to parse
					sort($origRules);
					return;
				}
				else {
					errorExit(LEX_SYN_CODE, SYN_QUOTE);
				}
			}
			else {
				errorExit(LEX_SYN_CODE, "SYN ERR - dash and right-arrow sign must not be divided\n");
			}
		}
		else {
			errorExit(LEX_SYN_CODE, SYN_QUOTE);
		}
	}
	/**
	* parseSymbols reads input symbols between apostrophes and pushes them into the symbol array
	*/
	function parseSymbols(&$token, &$inputFSM, &$origSymbols) {
		
		if ($token !== APOSTROPHE) {
			errorExit(LEX_SYN_CODE, SYN_QUOTE);
		}

		getToken($token, $inputFSM);
		// if after the initial apostrophe follows another one
		if ($token === APOSTROPHE) { // ''
			getToken($token, $inputFSM);
			if ($token === APOSTROPHE) { // ,'''
				array_push($origSymbols, "''"); 
			}
			else {
				errorExit(LEX_SYN_CODE, SYN_QUOTE);
			}
		}
		else {
			array_push($origSymbols, $token);
		}
		
		getToken($token, $inputFSM);
		if ($token === APOSTROPHE) {
			tokenCombo($token, $inputFSM);
			if ($token === COMMA) {
				tokenCombo($token, $inputFSM);
				parseSymbols($token, $inputFSM, $origSymbols);
				return; 
			}
			elseif ($token === RIGHT_CURLY) {
				$origSymbols = array_unique($origSymbols); //delete all duplicates
				sort($origSymbols); //sort the symbols alphabetically
				return;
			}
			else {
				errorExit(LEX_SYN_CODE, LEX_QUOTE);
			}
		} 
		else {
			errorExit(LEX_SYN_CODE, SYN_QUOTE);
		}
	}
	/**
	* parseStates reads input states and pushes them into the states array
	*/
	function parseStates(&$token, &$inputFSM, &$origStates){
		
		// scanning multicharacter state name
		$rollingDung = rollingDung($token, $inputFSM);

		// ({a, or ({a}
		cutSpaces($token, $inputFSM);
		if ($token === COMMA) {
			array_push($origStates, $rollingDung);
			tokenCombo($token, $inputFSM);
			parseStates($token, $inputFSM, $origStates);
			return;
		}
		elseif ($token === RIGHT_CURLY) {
			array_push($origStates, $rollingDung);
			$origStates = array_unique($origStates); //delete all duplicates
			sort($origStates); //sort the states alphabetically
			return;
		} 
		else {
			errorExit(LEX_SYN_CODE, LEX_QUOTE);
		}
	}
?>