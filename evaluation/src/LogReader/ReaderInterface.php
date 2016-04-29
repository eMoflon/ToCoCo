<?php

namespace App\LogReader;

/**
 * ReaderInterface
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
interface ReaderInterface {
	/**
	 * parses the provided mote output
	 * 
	 * @param string $logFile
	 * @param boolean $keepDebuggingMessages
	 * @return App\LogReader\Message[]
	 */
	public function parseMessages($logFile, $keepDebuggingMessages = true);
}