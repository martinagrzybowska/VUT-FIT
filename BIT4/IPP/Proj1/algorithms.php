<?php
/**
 *	Filename: algorithms.php
 *	Date: 26.2.2017
 *	Author: Martina Grzybowska, xgrzyb00@stud.fit.vutbr.cz
 * 	Project: IPP 16/17, FSM determinization
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	/**
	* ditchEps creates epsilon enclosures for each of the states in states set
	* and deletes epsilon rules
	*/
	function ditchEps(&$origStates, &$origRules, &$origFinalStates){

		$epsFreeRules = array();
		foreach ($origStates as $loop_state) { //p' ∈ Q
			
			$eps_enclosure = array($loop_state); //Q0 := {p};
			$eps_tmp = array(); //Qi-1;
			
			// creating epsilon enclosure for each state 
			while ($eps_tmp !== $eps_enclosure) { //until Qi != Qi-1
				$eps_tmp = $eps_enclosure; 
				foreach ($eps_tmp as $loop_tmp) { // q ∈ Qi-1
					foreach ($origRules as $loop_rule) { // q -> p' ∈ R
						if ($loop_rule->initState === $loop_tmp) {
							if ($loop_rule->inputSymbol === '') {
								if (!in_array($loop_rule->targetState, $eps_enclosure)) {
									array_push($eps_enclosure, $loop_rule->targetState);
								}
							}
						}
					}
				}
			}
			// ditching epsilon rules
			foreach ($eps_enclosure as $eps_state) { // for each p' ∈ Q do
				foreach ($origRules as $eps_rule) { // p'a -> q ∈ R
					if (($eps_rule->initState === $eps_state) and ($eps_rule->inputSymbol !== '')) { // p' is element of p enclosure
						if (!in_array($eps_rule, $epsFreeRules)) {
								array_push($epsFreeRules, $eps_rule);
						}
						$tmp = array();
						$tmp = clone $eps_rule;
						$tmp->initState = $loop_state;
							
						if (!in_array($tmp, $epsFreeRules)) {
							array_push($epsFreeRules, $tmp);
						}
					}
				}
			}
			// adding final states
			foreach ($eps_enclosure as $to_add) {
				if (in_array($to_add, $origFinalStates)) { // if one of the states in enclosure is a final state 
					if (!in_array($loop_state, $origFinalStates)) { // if the state is not a final state yet
						array_push($origFinalStates, $loop_state); // push it
					}
				}
			}
		}
		$origRules = $epsFreeRules;
		sort($origRules);
		sort($origStates);
		sort($origFinalStates);
	}
	/**
	* determinizeFSM takes the output from ditchEps and determinizes it
	*/
	function determinizeFSM(&$startState, &$origSymbols, &$origFinalStates, &$origRules, &$origStates) {
		$states_storage = array(array($startState)); //Qnew := {sd}; array of arrays
		$finalRules = array(); //Rd, array of objects
		$finalDFSMStates = array(); //Qd 
		$finalFinalStates = array(); //Fd
		$counter = 0;
		
		while(count($states_storage)) {
			
			// an array used to pick arrays from states_storage and iterate through them
			$s_iterator = array(); //Q'
			
			for($i = 0; $i < count($states_storage[$counter]); $i++) { 
				array_push($s_iterator, $states_storage[$counter][$i]); // Q' is an element of Qnew
			}

			unset($states_storage[$counter]); // Qnew := Qnew - {Q'};
			$counter++;
			
			// if there is more than one state in the array, it becomes a string a_a
			sort($s_iterator);
			$dfsm_aid = implode('_', $s_iterator);
			array_push($finalDFSMStates, $dfsm_aid);
			

			foreach ($origSymbols as $loop_sym) { 
			 	$s_pusher = array(); //Q''
			 	foreach ($s_iterator as $loop_iter) { // p is element of Q';
			 		foreach ($origRules as $R) { // pa -> q is element of R;
			 			if (($R->initState === $loop_iter) and ($R->inputSymbol === $loop_sym)) {
			 				if (!in_array($R->targetState, $s_pusher)){
			 					array_push($s_pusher, $R->targetState);	

			 				}
			 			}
			 		}
			 	}
		 		if (count($s_pusher)) { // Q'' is not empty;
		 			
		 			sort($s_pusher);
		 			$temp_aid2 = implode('_', $s_pusher);
		 			$temp_aid3 = FALSE;
		 			
		 			// if the state is not in the DFSM states array and it is not an epsilon
		 			if (!in_array($temp_aid2, $finalDFSMStates) and $temp_aid !== '') {
		 				// if it is not already in the queue
		 				foreach ($states_storage as $key => $inner_array) {
		 					if ($s_pusher === $inner_array) {
		 						$temp_aid3 = TRUE;
		 						break;
		 					}
		 				}
		 				if ($temp_aid3 == FALSE) {
		 					array_push($states_storage, $s_pusher);
		 				}
		 			}	 
		 			// create a new rule
		 			$temp_aid4 = FALSE;
		 			$rule = new InputRules();
		 			$rule->initState = implode("_", $s_iterator);
		 			$rule->inputSymbol = $loop_sym;
		 			$rule->targetState = implode("_", $s_pusher);
		 			
		 			// see if the rule is already exists
		 			foreach ($finalRules as $key) {
		 				if ($key->initState === $rule->initState) {
		 					if ($key->inputSymbol === $rule->inputSymbol) {
		 						if ($key->targetState === $rule->targetState) {
									$temp_aid4 = TRUE;
								}
							}
						}
		 			}
		 			// if it does not, push it
		 			if ($temp_aid4 == FALSE) {
		 				array_push($finalRules, $rule);
		 			}
		 		}
			}
			// if Q' is a part of F, add it to F
			if (array_intersect($s_iterator, $origFinalStates)) {
				if (!in_array(implode("_", $s_iterator), $finalFinalStates)) {
					array_push($finalFinalStates, implode("_", $s_iterator));
				}
			}
		}
	
		$origStates = $finalDFSMStates;
		$origRules = $finalRules;
		$origFinalStates = $finalFinalStates;
		sort($origRules);
		sort($origStates);
		sort($origFinalStates);
	}

	/**
	* wellSpecifiedFSM takes a determinizes FSM, ditches all non-ending states and
	* and creates a well specified fsm by creating a trap state called qFALSE
	*/
	function wellSpecifiedFSM(&$origStates, &$origRules, &$origSymbols, &$origFinalStates, &$CASE_SEN) {
	
	// ditching non-ending states
	$tmp_previous = array();
	$tmp_next = $origFinalStates; //Q0 = F;

	// looking for states which have a state from final states array as a traget state
	while ($tmp_next !== $tmp_previous) { //repeat until Qi = Qi-1
		$tmp_previous = $tmp_next; 
		foreach ($origRules as $loop_rule) { //qa -> p is element of R
			foreach ($origSymbols as $loop_sym) { // a is element of E
				foreach ($tmp_previous as $loop_state) { // p is element of Qi-1
					if (($loop_rule->targetState === $loop_state) and ($loop_rule->inputSymbol === $loop_sym)) {
						if (!in_array($loop_rule->initState, $tmp_next)) {
							array_push($tmp_next, $loop_rule->initState);
						}
					}
				}
			}
		}
	}
	$endingRules = array();
	//looking for rules in which the initial and target states belong to non-ending states
	foreach ($origRules as $loop_rule) {
		foreach ($origSymbols as $loop_sym) {
			foreach ($tmp_next as $loop_state) {
				if (($loop_rule->initState === $loop_state) and ($loop_rule->inputSymbol === $loop_sym)) {
					if (in_array($loop_rule->targetState, $tmp_next)) {
						array_push($endingRules, $loop_rule);
					}
				}
			}
		}
	}
	if ($CASE_SEN) {
	 	$qFalse = "qfalse";
	}
	else {
		$qFalse = "qFALSE";
	}
	array_push($tmp_next, $qFalse);
	$comRules = $endingRules;
	// creating qFALSE rules
	foreach ($origSymbols as $loop_sym) {
		foreach ($tmp_next as $first_state) {
			$isThere = FALSE;
			foreach ($endingRules as $loop_rule) {	
				foreach ($tmp_next as $sec_state) {
					//chech if the state has a rule with sym in it
					if ($loop_rule->initState === $first_state) {
						if ($loop_rule->inputSymbol === $loop_sym) {
							if ($loop_rule->targetState === $sec_state) {
								$isThere = TRUE;
							}
						}
					}
				}
			}
			if (!$isThere) {
				$single_rule = new InputRules();
				$single_rule->initState = $first_state;
				$single_rule->inputSymbol = $loop_sym;
				$single_rule->targetState = $qFalse;
				$exists = FALSE;
				//if the rule already exists in the new array, don't push it
				foreach ($comRules as $key) {
	 				if ($key->initState === $single_rule->initState) {
	 					if ($key->inputSymbol === $single_rule->inputSymbol) {
	 						if ($key->targetState === $single_rule->targetState) {
								$exists = TRUE;
							}
						}
					}
	 			}
				if (!$exists) {
					array_push($comRules, $single_rule);
				}
			}
		}
	}
	//in case of the starting state being non-ending, make it into qFALSE
	if (!in_array($startState, $tmp_next)) {
		$startState = $qFalse;
	}

	$origRules = $comRules;
	$origStates = $tmp_next;
	sort($origRules);
	sort($origStates);
	}
?>