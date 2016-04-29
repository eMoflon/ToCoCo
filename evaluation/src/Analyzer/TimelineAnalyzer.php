<?php

namespace App\Analyzer;

use Colors\RandomColor;

/**
 * Timeline Analyzer
 * 
 * Not all testbeds create sorted and readable outputs. With the Timeline Analzyer
 * you can create a nice-looking searchable HTML representation of the mote outputs.
 *
 * REMARK: this is not directly an analyzer but the same implemented interface makes
 * the development of the evaluation cli-script much easier
 *
 * @author Tobias Petry <tobias_petry@hotmail.com>
 */
class TimelineAnalyzer implements AnalyzerInterface {
	/**
	 * analyzes the mote outputs
	 *
	 * @param App\LogReader\Message[]
	 * @return string
	 */
	public function analyze(array $messages) {
		ob_start();
		?>
		<!DOCTYPE html>
		<html lang="en">
			<head>
				<meta charset="utf-8">
				<meta http-equiv="X-UA-Compatible" content="IE=edge">
				<meta name="viewport" content="width=device-width, initial-scale=1">
				<title>Mote Output Timeline</title>
				<link href="http://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.5/css/bootstrap.min.css" rel="stylesheet">
				<link href="http://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.3.0/css/font-awesome.min.css" rel="stylesheet">
				<script src="http://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.4/jquery.min.js"></script>
				<script src="http://cdnjs.cloudflare.com/ajax/libs/jquery-throttle-debounce/1.1/jquery.ba-throttle-debounce.min.js"></script>
				<!--[if lt IE 9]>
					<script src="http://cdnjs.cloudflare.com/ajax/libs/html5shiv/3.7.2/html5shiv.min.js"></script>
					<script src="http://cdnjs.cloudflare.com/ajax/libs/respond.js/1.4.2/respond.min.js"></script>
				<![endif]-->
				<script>
					$(function() {
						$("#regexfilter").on("input", $.debounce(250, function() {
							var regexp = new RegExp($("#regexfilter").val(), "i");
							var $tbody = $("table tbody").detach();
							$tbody.find("tr").each(function(_num, tr) {
								if($(tr).text().replace(/(\r)?\n/g, " ").match(regexp))
									$(tr).removeClass("hidden");
								else
									$(tr).addClass("hidden");
							});
							$("table").append($tbody);
							$("#regexfilter-results").html($("table tbody tr:visible").length + " log messages");
						})).trigger("input");
					});
				</script>
			</head>
			<body>
				<div class="container">
					<h1>Mote Output Timeline</h1>
					<div class="col-md-4 col-md-offset-8" style="padding-right:0px">
						<div class="input-group">
							<input type="text" class="form-control" id="regexfilter" placeholder="Regular Expressions Filter">
							<span class="input-group-addon" id="regexfilter-results"><i class="fa fa-spinner fa-spin"></i></span>
						</div>
					</div>
					<table class="table table-condensed">
						<thead>
							<tr>
								<th>Time</th>
								<th>Logid</th>
								<th>Mote</th>
								<th>Message</th>
								<th class="hidden">Searchable Cooja Output</th>
							</tr>
						</thead>
						<tbody>
							<?php $moteColors = [] ?>
							<?php foreach($messages as $message): ?>
								<?php if(!isset($moteColors[$message->getMoteid()])): ?>
									<?php mt_srand($message->getMoteid()) /* the random color of a mote should be the same every time */ ?>
									<?php $moteColors[$message->getMoteid()] = RandomColor::one(["luminosity" => "light"]) ?>
								<?php endif ?>
								<tr style="background-color:<?= $moteColors[$message->getMoteid()] ?>;">
									<td><?= $this->_formatPointInTime($messages[0]->getTimestamp(), $message->getTimestamp()) ?></td>
									<td><?= $message->getLogid() ?></td>
									<td><?= $message->getMoteid() ?></td>
									<td><?= str_replace(",", ",<wbr>", htmlspecialchars($message->getMessage(), ENT_QUOTES, "UTF-8")) ?></td>
									<td class="hidden">ID:<?= $message->getMoteid() ?> <?= htmlspecialchars($message->getMessage(), ENT_QUOTES, "UTF-8") ?></td>
								</tr>
							<?php endforeach ?>
						</tbody>
					</table>
				</div>
			</body>
		</html>
		<?php
		return ob_get_clean();
	}
	
	/**
	 * serializes the analyzed results to a html file
	 *
	 * @param string $filename
	 * @param string $data
	 */
	public function serialize($filename, $data) {
		file_put_contents($filename, $data);
	}
	
	/**
	 * gets serialization filetype
	 *
	 * @return string filetype
	 */
	public function serializationType() {
		return "html";
	}
	
	/**
	 * merges several serialized evaluations
	 *
	 * @param string $filename
	 * @param array $serializationFiles
	 */
	public function merge($filename, array $serializationFiles) {
		throw new Exception("merge operation makes no sense for this analyzer");
	}
	
	/**
	 * formats time point
	 * 
	 * @param float $startTime
	 * @param float $pointInTime
	 * @return string
	 */
	protected function _formatPointInTime($startTime, $pointInTime) {
		$diff = $pointInTime - $startTime;
		
		$diff -= ($hours = floor($diff / 3600)) * 3600;
		$diff -= ($minutes = floor($diff / 60)) * 60;
		$diff -= ($seconds = floor($diff));
		
		return sprintf("%02d:%02d:%02d.%03d", $hours, $minutes, $seconds, $diff * 1000);
	}
}