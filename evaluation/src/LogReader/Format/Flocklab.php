<?php

namespace App\LogReader\Format;

use App\LogReader\Exception;
use App\LogReader\Message;
use App\LogReader\ReaderInterface;
use App\LogReader\Helper\SortTrait;
use App\LogReader\App\LogReader;

/**
 * Flocklab LogReader
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class Flocklab implements ReaderInterface {
	use SortTrait;
	
	/**
	 * parses the provided mote output
	 * 
	 * @param string $logFile
	 * @param boolean $keepDebuggingMessages
	 * @return App\LogReader\Message[]
	 */
	public function parseMessages($logFile, $keepDebuggingMessages = true) {
		$handle = fopen($logFile, "r");
		if(!$handle)
			throw new Exception("failed opening '" . $logFile . "'");
		
		$logid = 0;
		$messages = array();
		while($line = fgetcsv($handle, 0, ",")) {
			if(!is_numeric($line[0]))
				continue;
			
			$timestamp = (float) $line[0];
			$logmessage = implode(",", array_slice($line, 4));
			$logid++;
			if($keepDebuggingMessages || strpos($logmessage, "DEBUG:") !== 0) {
				$messages[] = new Message($timestamp, intval($line[2]), $logmessage, $logid);
			}
			
			if(strpos($logmessage, "ERROR") === 0) {
				trigger_error("'$logmessage' @ moteid={$line[2]}, logid=$logid", E_USER_WARNING);
			}
		}
		
		$this->_sortMessages($messages);
		
		// sometimes flocklabs log have serious errors with empty mote output preceding the programmed
		// runtime by several minutes --> try to remove these log messages
		while(true) {
			for($lookupWindow = 0; $lookupWindow < 10; $lookupWindow++) {
				// idea: all motes start at the same time, if two messages at the beginning have a large timeframe between
				// them, the may be unwanted messages. (Edge Case:) This will fail if only one mote would be started...
				if(abs($messages[$lookupWindow]->getTimestamp() - $messages[$lookupWindow + 1]->getTimestamp()) >= 120) {
					array_splice($messages, $lookupWindow, 1);
					continue 2;
				}	
			}
			
			break;
		}
		
		// Very long output lines will be split by Flocklab. This is not really detectable generally for the reader but evaluation
		// output is expected to be single line. So it can be fixed for evaluation line splitting if a line without evaluation prefix
		// is surrounded by two lines with evaluation output
		for($i = 0; $i < count($messages); $i++) {
			if(substr($messages[$i]->getMessage(), 0, 1) !== "[" && substr($messages[$i]->getMessage(), 0, strlen("DEBUG:")) !== "DEBUG:") {
				// search for preceeding message
				for($j1 = $i - 1; $j1 >= 0; $j1--) {
					if($messages[$j1]->getMoteid() == $messages[$i]->getMoteid()) {
						if(substr($messages[$j1]->getMessage(), 0, strlen("[evaluation]")) !== "[evaluation]")
							continue 2;
						
						break; // preceeding message is in $messages[$j1]
					}
				}
				if($j1 === -1)
					continue;
				
				// search for succeeding message
				for($j2 = $i + 1; $j2 < count($messages); $j2++) {
					if($messages[$j2]->getMoteid() == $messages[$i]->getMoteid()) {
						if(substr($messages[$j2]->getMessage(), 0, strlen("[evaluation]")) !== "[evaluation]")
							continue 2;
				
						break; // succeeding message is in $messages[$j1]
					}
				}
				if($j2 === count($messages))
					continue;
				
				
				// combine messages
				$messages[$j1] = new Message(
					$messages[$j1]->getTimestamp(),
					$messages[$j1]->getMoteid(),
					$messages[$j1]->getMessage() . $messages[$i]->getMessage(),
					$messages[$j1]->getLogid()
				);
				unset($messages[$i]);
				$messages = array_values($messages);
				$i = max(0, $i - 1);
			}
		}
		
		return $messages;
	}
}