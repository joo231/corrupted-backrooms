$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$Root = Split-Path -Parent $PSScriptRoot
$Raw = Join-Path $Root 'RawArt'
$Ua = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36'
$Headers = @{
    'User-Agent' = $Ua
    'Accept'     = '*/*'
}

function Get-PhJson([string]$Url) {
    return Invoke-RestMethod -Uri $Url -Headers $Headers -TimeoutSec 120
}

function Save-Url([string]$Url, [string]$Dest) {
    $dir = Split-Path -Parent $Dest
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir | Out-Null
    }
    if ((Test-Path $Dest) -and ((Get-Item $Dest).Length -gt 1024)) {
        Write-Host "skip $Dest"
        return
    }
    Write-Host "GET $Url"
    $tmp = "$Dest.download"
    Invoke-WebRequest -Uri $Url -Headers $Headers -OutFile $tmp -TimeoutSec 300
    Move-Item -Force $tmp $Dest
    Write-Host (" saved {0} ({1} bytes)" -f $Dest, (Get-Item $Dest).Length)
}

function Save-FbxModel([string]$AssetId, [string]$Res = '2k') {
    $files = Get-PhJson "https://api.polyhaven.com/files/$AssetId"
    $fbx = $files.fbx.$Res.fbx
    if (-not $fbx) {
        throw "No FBX $Res for $AssetId"
    }
    $folder = Join-Path $Raw "Models\$AssetId"
    Save-Url $fbx.url (Join-Path $folder "$AssetId.fbx")
    foreach ($rel in $fbx.include.PSObject.Properties.Name) {
        $meta = $fbx.include.$rel
        $safeRel = $rel -replace '/', '\'
        Save-Url $meta.url (Join-Path $folder $safeRel)
    }
}

function Save-Texture([string]$AssetId, [string]$Res = '2k') {
    $files = Get-PhJson "https://api.polyhaven.com/files/$AssetId"
    $folder = Join-Path $Raw "Textures\$AssetId"
    $map = @{
        Diffuse = 'diff.jpg'
        nor_gl  = 'nor.jpg'
        Rough   = 'rough.jpg'
        AO      = 'ao.jpg'
    }
    foreach ($key in $map.Keys) {
        $node = $files.$key
        if (-not $node) { continue }
        $resNode = $node.$Res
        if (-not $resNode) { continue }
        $fmt = $resNode.jpg
        if (-not $fmt) { $fmt = $resNode.png }
        if (-not $fmt) { continue }
        Save-Url $fmt.url (Join-Path $folder ("{0}_{1}" -f $AssetId, $map[$key]))
    }
}

function Save-Hdri([string]$AssetId, [string]$Res = '2k') {
    $files = Get-PhJson "https://api.polyhaven.com/files/$AssetId"
    $hdr = $files.hdri.$Res.hdr
    if ($hdr) {
        Save-Url $hdr.url (Join-Path $Raw "HDRI\${AssetId}_${Res}.hdr")
    }
}

$models = @(
    'marble_bust_01',
    'horse_statue_01',
    'lion_head',
    'horse_head',
    'bull_head',
    'antique_ceramic_vase_01',
    'ceramic_vase_02',
    'brass_vase_01'
)

foreach ($m in $models) {
    Write-Host "==== model $m ===="
    Save-FbxModel $m
}

foreach ($t in @('marble_01', 'marble_tiles')) {
    Write-Host "==== texture $t ===="
    Save-Texture $t
}

Write-Host '==== hdri ===='
Save-Hdri 'kloofendal_38d_partly_cloudy'
Write-Host 'DONE'
