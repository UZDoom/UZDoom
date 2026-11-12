<?php

$repo = "UZDoom/UZDoom";

$release = $_GET["r"];
if (empty($release) || str_contains($release, '/')) die();

$asset = $_GET["f"];
if (empty($asset) || str_contains($asset, '/')) die();

if ($release === "latest") {
    $url = "https://github.com/".$repo."/releases/".$release."/download/".$asset;
} else {
    $url = "https://github.com/".$repo."/releases/download/".$release."/".$asset;
}

header("Location: " . $url, true, 302);

exit();

?>
