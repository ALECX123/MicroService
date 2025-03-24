## MicroService简介
用作后台服务，轮询监听启动常驻软件
## 安装
```bash
MicroService install
```  
## 配置
```ini
;process.ini
[setting]
log=false
; comments
[proxy]
ExeName=proxy
Parameters=http -t tcp -p "0.0.0.0:10809" -C proxy.crt -K proxy.key
Env=D:\workbench\goproxy
Show=true
```
## 卸载
```bash
MicroService remove
```