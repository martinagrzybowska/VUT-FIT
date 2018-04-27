<?php
/**
 *	Filename: dka.php
 *	Date: 26.2.2017
 *	Author: Martina Grzybowska, xgrzyb00@stud.fit.vutbr.cz
 * 	Project: IPP 16/17, FSM determinization
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	include('parameters.php');
	include('parser.php');
	include('functions.php');
	include('algorithms.php');

	mb_internal_encoding("UTF-8");
	mb_regex_encoding("UTF-8");

	// parameter flags
	$INPUT = FALSE; // --input-filename
	$OUTPUT = FALSE; // --output-filename
	$NO_EPSILON = FALSE; // -e, --no-epsilon-rules
	$DETERM = FALSE; // -d, --determinization
	$CASE_SEN = FALSE; // -i, --case-insensitive
	$WSFA = FALSE; // --wsfa

	// constants
	define("LEFT_CURLY", "{");
	define("RIGHT_CURLY", "}");
	define("LEFT_ROUND", "(");
	define("RIGHT_ROUND", ")");
	define("COMMA", ",");
	define("APOSTROPHE", "'");
	define("RIGHT_ARROW", ">");
	define("DASH", "-");
	define("UNDERSCORE","_");
	define("EPSILON", "eps");
	
	// error codes and quotes
	define("LEX_SYN_CODE", 40);
	define("SEM_CODE", 41);
	define("OPT_CODE", 1);
	define("LEX_QUOTE", "LEX ERROR - invalid character in state name\n");
	define("SYN_QUOTE", "SYN ERROR - unexpected character\n");
	define("OPT_QUOTE", "OPT ERROR - something wrong with command line arguments\n");

	// input rules object structure
	class InputRules {
		public $initState;
		public $inputSymbol;
		public $targetState;
	}

	// I/O controlers
	$inputFSM = '';
	$outputFile = '';
	
	// global variables
	$origStates = array();
	$origSymbols = array();
	$origRules = array();
	$startState = '';
	$origFinalStates = array();
	
	// get flags
	getParameters($INPUT, $OUTPUT, $NO_EPSILON, $DETERM, $CASE_SEN, $WSFA, $inputFSM, $outputFile, $argv);	
	
	//if -i / --case-insensitive is on, convert everything to lowercase
	if ($CASE_SEN) {
		$inputFSM = mb_strtolower($inputFSM);
	}
	
	//check syntax
	parsingAscent($inputFSM, $origStates, $origSymbols, $origRules, $startState, $origFinalStates);

	if ($DETERM) {
		ditchEps($origStates, $origRules, $origFinalStates);
		determinizeFSM($startState, $origSymbols, $origFinalStates, $origRules, $origStates);
	}
	elseif ($NO_EPSILON) {
		ditchEps($origStates, $origRules, $origFinalStates);
	}
	elseif ($WSFA) {
		ditchEps($origStates, $origRules, $origFinalStates);
		determinizeFSM($startState, $origSymbols, $origFinalStates, $origRules, $origStates);
		wellSpecifiedFSM($origStates, $origRules, $origSymbols, $origFinalStates, $CASE_SEN);
	}
	
	printFSM($origStates, $origSymbols, $origRules, $startState, $origFinalStates, $outputFile);

?>