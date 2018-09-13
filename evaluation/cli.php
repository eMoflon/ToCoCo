#!/usr/bin/env php
<?php

ini_set("memory_limit", "512M");

if(!file_exists(__DIR__ . "/vendor/autoload.php"))
	die("You have to load all evaluation dependencies with 'composer install'\n");
require_once __DIR__ . "/vendor/autoload.php";

// cli param definition and handling
$climate = new League\CLImate\CLImate();
$climate->arguments->add([
	"testbed" => [
		"longPrefix" => "testbed",
		"description" => "testbed the input file was created from: flocklab|tiz|cooja"
	],
	"source" => [
		"longPrefix" => "source",
		"description" => "message log export from the testbed"
	],
	"evaluation" => [
		"longPrefix" => "evaluation",
		"description" => "evaluation to run on the source file: prr|energy-milliamperehour|energy-ticks|timeline|powercalibration|graph-routing|graph-neighborhood|graph-text|stretch-plot",
		"required" => true
	],
	"open" => [
		"longPrefix" => "open",
		"description" => "opens the evaluation result",
		"noValue" => true
	],
	"destination" => [
		"longPrefix" => "destination",
		"description" => "exports the evaluation results to the destination file"
	],
	"graph-minute" => [
		"longPrefix" => "graph-minute",
		"description" => "runtime minute the graph image should be created for"
	],
	"keep-debugging-messages" => [
		"longPrefix" => "keep-debugging-messages",
		"description" => "do not throw away debugging messages (will have higher memory footprint and runtime)",
		"noValue" => true
	],
	"merge" => [
		"longPrefix" => "merge",
		"description" => "merges several evaluation results (seperate the input files by comma)",
	],
]);
try {
	$climate->arguments->parse();
} catch(\Exception $e) {
	$climate->error($e->getMessage());
	$climate->usage();
	exit();
}

$testbed = [
	"flocklab" => new App\Testbed\Flocklab(),
	"tiz" => new App\Testbed\Tiz(),
	"cooja" => new App\Testbed\Cooja(),
];
$evaluation = [
	"prr" => new App\Analyzer\PrrAnalyzer(),
	"energy-milliamperehour" => new App\Analyzer\EnergyAnalyzer(App\Analyzer\EnergyAnalyzer::ENERGY_MILLIAMPEREHOUR),
	"energy-ticks" => new App\Analyzer\EnergyAnalyzer(App\Analyzer\EnergyAnalyzer::ENERGY_TICKS),
	"timeline" => new App\Analyzer\TimelineAnalyzer(),
	"powercalibration" => new App\Analyzer\PowercalibrationAnalyzer(),
	"graph-routing" => new App\Analyzer\RoutingGraphAnalyzer($climate->arguments->get("graph-minute"), (isset($testbed[$climate->arguments->get("testbed")])) ? $testbed[$climate->arguments->get("testbed")]->getMotePositions() : []),
	"graph-neighborhood" => new App\Analyzer\NeighborhoodGraphAnalyzer($climate->arguments->get("graph-minute"), (isset($testbed[$climate->arguments->get("testbed")])) ? $testbed[$climate->arguments->get("testbed")]->getMotePositions() : []),
	"graph-text" => new App\Analyzer\BridgeGraphAnalyzer($climate->arguments->get("graph-minute"),(isset($testbed[$climate->arguments->get("testbed")]))? $testbed[$climate->arguments->get("testbed")]->getMotePositions():[]),
	"stretch-plot" => new App\Analyzer\StretchAnalyzer(),
];

// better error messages on cli output
error_reporting(E_ALL);
set_error_handler(function($errno, $errstr, $errfile, $errline) use($climate) {
	if (error_reporting() === 0)
		return false;
	
	if(array_search($errno, [E_WARNING, E_NOTICE, E_USER_WARNING, E_USER_NOTICE], true) !== false) {
		$climate->comment("$errstr in $errfile:$errline");
	} else {
		$climate->error("$errstr in $errfile:$errline");
		ob_start();
		debug_print_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS);
		$climate->error(ob_get_clean());
		die();
	}
});
set_exception_handler(function(Exception $e) use($climate) {
	$climate->error(sprintf("%s in %s:%d", $e->getMessage(), $e->getFile(), $e->getLine()));
});

// test evaluation
if(!isset($evaluation[$climate->arguments->get("evaluation")])) {
	$climate->error("unknown evaluation '{$climate->arguments->get("evaluation")}'");
	$climate->usage();
	exit();
}

if(!$climate->arguments->defined("merge")) {
	// parse testbed log messages
	if(!isset($testbed[$climate->arguments->get("testbed")])) {
		$climate->error("unknown testbed '{$climate->arguments->get("testbed")}'");
		$climate->usage();
		exit();
	}
	$messages = $testbed[$climate->arguments->get("testbed")]->getLogReader()->parseMessages($climate->arguments->get("source"), $climate->arguments->defined("keep-debugging-messages"));
	
	// run evaluation
	if($climate->arguments->get("evaluation") === "graph-routing" && !$climate->arguments->defined("graph-minute")) {
		$climate->error("--graph-minute has to be defined for graph evaluation");
		$climate->usage();
		exit();
	}
	$data = $evaluation[$climate->arguments->get("evaluation")]->analyze($messages);
	
	// serialize to a temporary file
	$filename = sprintf("%s/%s.%s", sys_get_temp_dir(), uniqid(), $evaluation[$climate->arguments->get("evaluation")]->serializationType());
	$evaluation[$climate->arguments->get("evaluation")]->serialize($filename, $data);
} else {
	// serialize to a temporary file
	$filename = sprintf("%s/%s.%s", sys_get_temp_dir(), uniqid(), $evaluation[$climate->arguments->get("evaluation")]->serializationType());
	$evaluation[$climate->arguments->get("evaluation")]->merge($filename, explode(',', $climate->arguments->get("merge")));
}

// save evaluation to requested file
if($climate->arguments->defined("destination")) {
	file_put_contents($climate->arguments->get("destination"), file_get_contents($filename));
}

// open evaluation results
if($climate->arguments->defined("open")) {
	exec("open " . escapeshellarg($filename) . " > /dev/null 2>&1 &");
}

// print execution time
$climate->info("evaluated in " . round(microtime(true) - $_SERVER['REQUEST_TIME_FLOAT'], 3) . "s");
