<?php
$in1 = 'grfbuilder_empty.ts';
$in2 = 'grfbuilder_zh_TW.xml';
$out = 'grfbuilder_zh_TW.ts';

$fin1 = fopen($in1, 'r');
$fin2 = fopen($in2, 'r');
$fout = fopen($out, 'w');

// Read line by line
while(!feof($fin1)) {
	$lin1 = fgets($fin1, 4096);
	$lin2 = fgets($fin2, 4096);
	if ($lin1==$lin2) {
		fwrite($fout, $lin1);
		continue;
	}
	fwrite($fout, $lin1);
	fwrite($fout, str_replace('source', 'translation', $lin2));
	$lin1 = fgets($fin1, 4096);
	$lin2 = fgets($fin2, 4096);
}

