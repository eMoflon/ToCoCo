<?php

namespace App\Analyzer\Helper;

/**
 * Serialize Trait
 * 
 * Will serialize the provided data in a generic way to csv files.
 * The data is required to have a numeric index which will be interpreted
 * as the minute the analyzed information stands for.
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
trait SerializeTrait {
	/**
	 * serializes the analyze results to a csv file
	 * 
	 * @param string $filename
	 * @param array $values
	 */
	public function serialize($filename, array $data) {
		$appendDataValue = function(&$fields, $dataName, $dataValue) use(&$appendDataValue) {
			if(!is_array($dataValue)) {
				$fields[$dataName] = $dataValue;
			} else {
				foreach($dataValue as $subDataName => $subDataValue) {
					$appendDataValue($fields, $dataName . "_" . $subDataName, $subDataValue);
				}
			}
		};
	
		$handle = fopen($filename, "w+");
		$printedHeader = false;
		foreach($data as $dataValues) {
			$fields = [];
			foreach($dataValues as $dataName => $dataValue)
				$appendDataValue($fields, $dataName, $dataValue);
				
			if(!$printedHeader) {
				fputcsv($handle, array_keys($fields));
				$printedHeader = true;
			}
				
			fputcsv($handle, $fields);
		}
	}
	
	/**
	 * unserializes an serialized results csv file
	 * 
	 * @param string $filename
	 * @return array
	 */
	protected function _unserialize($filename, $skipAggregates = true) {
		$handle = fopen($filename, "r");
		$header = null;
		
		$data = [];
		while($line = fgetcsv($handle, 0)) {
			if($header === null) {
				$header = $line;
				continue;
			}
			
			$unserialized = array_combine($header, $line);
			foreach($unserialized as &$dataValue)
				$dataValue = str_replace(",", ".", $dataValue);
			
			if(!$skipAggregates || ($unserialized["moteid"] != -1 && $unserialized["runtime"] != -1))
				$data[] = $unserialized;
		}
		
		return $data;
	}
	
	protected function _unserializeNoRuntime($filename, $skipAggregates = true) {
		$handle = fopen($filename, "r");
		$header = null;
		
		$data = [];
		while($line = fgetcsv($handle, 0)) {
			if($header === null) {
				$header = $line;
				continue;
			}
			
			$unserialized = array_combine($header, $line);
			foreach($unserialized as &$dataValue)
				$dataValue = str_replace(",", ".", $dataValue);
			
			if(!$skipAggregates || ($unserialized["moteid"] != -1))
				$data[] = $unserialized;
		}
		
		return $data;
	}

	/**
	 * gets serialization filetype
	 *
	 * @return string filetype
	 */
	public function serializationType() {
		return "csv";
	}
}