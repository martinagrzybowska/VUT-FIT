<?php
/**
 *	Filename: parameters.php
 *	Date: 26.2.2017
 *	Author: Martina Grzybowska, xgrzyb00@stud.fit.vutbr.cz
 * 	Project: IPP 16/17, FSM determinization
  * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	/**
	* getParameters reads and parses command line arguments
	*/
	function getParameters(&$INPUT, &$OUTPUT, &$NO_EPSILON, &$DETERM, &$CASE_SEN, &$WSFA, &$inputFSM, &$outputFile, &$argv) {

		// delete script name
		unset($argv[0]);
		
		foreach ($argv as $opt_name) {

			// --help  
			if ($opt_name === '--help') {
				if (count($argv) > 1) {
					errorExit(OPT_CODE, OPT_QUOTE);
				} 
				else {
					printHelp();
					exit(0);
				}
			} 
			// --input=filename
			elseif (mb_substr($opt_name, 0, 8) === '--input=') {
				if ($INPUT == FALSE) {
					$original_path = mb_substr($opt_name, 8);
					if (!is_file($original_path)) {
						errorExit(2, "FILE ERR - the file does not exist\n");
					}
					else {
						$INPUT = TRUE;
					}
				}
				else {
					errorExit(OPT_CODE, OPT_QUOTE);
				}
			}
			// --output=filename
			elseif (mb_substr($opt_name, 0, 9) === '--output=') {
				if ($OUTPUT == FALSE) {
					$OUTPUT = TRUE;
					$output_filename = mb_substr($opt_name, 9);
				}
				else {
					errorExit(OPT_CODE, OPT_QUOTE);
				}
			}
			// -e, --no-epsilon-rules 
			elseif (($opt_name === '-e') or ($opt_name === '--no-epsilon-rules')) {
				if (($DETERM == FALSE) and ($NO_EPSILON == FALSE)) {
					$NO_EPSILON = TRUE;
				}
				else {
					errorExit(OPT_CODE, OPT_QUOTE);
				}
			}
			// -d, --determinization
			elseif (($opt_name === '-d') or ($opt_name === '--determinization')) {
				if ($NO_EPSILON == FALSE && $DETERM == FALSE) {
					$DETERM = TRUE;
				}
				else {
					errorExit(OPT_CODE, OPT_QUOTE);
				}
			}
			// -i, --case-insensitive 
			elseif (($opt_name === '-i') or ($opt_name === '--case-insensitive')) {
				if ($CASE_SEN == FALSE) {
					$CASE_SEN = TRUE;
				}
				else {
					errorExit(OPT_CODE, OPT_QUOTE);
				}
			}
			elseif ($opt_name === "--wsfa") {
				if (($WSFA == FALSE) and ($DETERM == FALSE) and ($NO_EPSILON == FALSE)) {
					$WSFA = TRUE;
				}
				else {
					errorExit(OPT_CODE, OPT_QUOTE);
				}
			}
			elseif (count($argv) == 0) {
				break;
			}
			// the rest
			else {
				errorExit(OPT_CODE, OPT_QUOTE);
			}
		}
		
		// if --input parameter is not set
		if ($INPUT == FALSE) {
			$original_path = "php://stdin";
		}
		
		$inputFSM = file_get_contents($original_path, 'r');

		if ($inputFSM == FALSE) {
			errorExit(2, "FILE ERR - unable to open input file\n");
		}

		//should the file be created on windows
		$inputFSM = preg_replace("/\r\n/", "\n", $inputFSM);

		// set output
		if ($OUTPUT == TRUE) {
			$outputFile = fopen($output_filename, "w");
			if ($outputFile == FALSE) {
				errorExit(3, "FILE ERR - unable to create output file\n");
			}
		}
		return;
	}
?>