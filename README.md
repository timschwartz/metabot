# metabot
Virtual Reality bot with scripting.

## Bot configuration files (*.bot)

```
{
  "server": "beta.vrsites.com",
  "port": 5567,
  "name": "MyBot",
  "owner": "Me",
  "avatar_file": "/full/path/to/avatars/bot.avatar",
  "room": "http://www.janusvr.com/index.html",
  "position": {
    "pos":"1.5 0 -5.0",
    "dir":"0 0 0",
    "view_dir":"0 0 0",
    "up_dir":"0 0 0",
    "head_pos":"0 0 0"
   }
}
```

## Avatar files (*.avatar)

```
<FireBoxRoom>
  <Assets>
    <AssetObject id=^body^ src=^http://avatars.vrsites.com/chibii/body_male.obj^ mtl=^http://avatars.vrsites.com/chibii/mtls3/body_male13.mtl^ />
    <AssetObject id=^head^ src=^http://avatars.vrsites.com/chibii/head_male.obj^ mtl=^http://avatars.vrsites.com/chibii/mtls3/head_male13.mtl^ />
  </Assets>
  <Room>
    <Ghost id=^BOTNAME^ js_id=^2^ scale=^1.00000 1.00000 1.00000^ head_id=^head^ head_pos=^0.000000 0.750000 0.000000^ body_id=^body^ />
  </Room>
</FireBoxRoom>

```

Note that `Ghost id` is set to `BOTNAME`. This will be replaced with whatever name is defined in the .bot file.
