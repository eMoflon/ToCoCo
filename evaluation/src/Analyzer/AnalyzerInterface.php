<?php

namespace App\Analyzer;

/**
 * Analyzer Interface
 * 
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
interface AnalyzerInterface {
	/**
	 * analyzes the mote outputs
	 *
	 * @param App\LogReader\Message[]
	 * @return mixed
	 */
	public function analyze(array $messages);
	
	/**
	 * serializes the analyze results to a file
	 * 
	 * @param string $filename
	 * @param mixed $value
	 */
	public function serialize($filename, $value);
	
	/**
	 * gets serialization filetype
	 * 
	 * @return string filetype
	 */
	public function serializationType();
	
	/**
	 * merges several serialized evaluations
	 *
	 * @param string $filename
	 * @param array $serializationFiles
	 */
	public function merge($filename, array $serializationFiles);
}