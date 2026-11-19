param(
  [Parameter(Mandatory = $true)][string]$ProxyTrace,
  [Parameter(Mandatory = $true)][string]$GatewayTrace
)

$sent = @(Import-Csv -LiteralPath $ProxyTrace)
$received = @(Import-Csv -LiteralPath $GatewayTrace)
$counts = @{}

function Get-FrameKey($row) {
  return "$($row.sysid),$($row.compid),$($row.seq),$($row.msgid),$($row.len),$($row.checksum),$($row.magic)"
}

foreach ($row in $sent) {
  $key = Get-FrameKey $row
  if ($counts.ContainsKey($key)) { $counts[$key]++ }
  else { $counts[$key] = 1 }
}

$extra = 0
foreach ($row in $received) {
  $key = Get-FrameKey $row
  if ($counts.ContainsKey($key) -and $counts[$key] -gt 0) { $counts[$key]-- }
  else { $extra++ }
}

$missing = 0
foreach ($count in $counts.Values) { $missing += $count }
$pass = ($missing -eq 0 -and $extra -eq 0)
"UPSTREAM TRACE RESULT: sent=$($sent.Count) gateway_received=$($received.Count) missing=$missing extra=$extra pass=$([int]$pass)"
if ($pass) { exit 0 }
exit 1
