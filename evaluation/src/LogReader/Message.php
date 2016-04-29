<?php

namespace App\LogReader;

/**
 * Message
 * 
 * A single mote output message
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class Message {
	/**
	 * @var float
	 */
	protected $_timestamp;
	
	/**
	 * @var int
	 */
	protected $_moteid;
	
	/**
	 * @var string
	 */
	protected $_message;
	
	/**
	 * @var int
	 */
	protected $_logid;
	
	/**
	 * constructs a new message
	 * 
	 * @param float $timestamp
	 * @param int $moteid
	 * @param string $message
	 * @param int $logid
	 */
	public function __construct($timestamp, $moteid, $message, $logid) {
		$this->_timestamp = $timestamp;
		$this->_moteid = $moteid;
		$this->_message = $message;
		$this->_logid = $logid;
	}
	
	/**
	 * get the timestamp of the log message
	 * 
	 * @return float
	 */
	public function getTimestamp() {
		return $this->_timestamp;
	}
	
	/**
	 * get the moteid of the log message
	 * 
	 * @return int
	 */
	public function getMoteid() {
		return $this->_moteid;
	}
	
	/**
	 * get the message of the log message
	 * @return string
	 */
	public function getMessage() {
		return $this->_message;
	}
	
	/**
	 * get the logid of the log message
	 * 
	 * @return int
	 */
	public function getLogid() {
		return $this->_logid;
	}
}