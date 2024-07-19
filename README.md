包括实验环境安装 VMWare和Ubuntu20.04安装、mininet 安装

实验设计思路、项目内容、项目测试，实际源码。


实现：使用 C 语言实现最简单的 HTTP 服务器

同时支持 HTTP（80 端口）和 HTTPS（443 端口）使用两个线程分别监听各自端口
   
只需支持 GET 方法，解析请求报文，返回相应应答及内容
   
 支持的状态码：

 ![06b09e5cdb68152d19faa5286b6a85b](https://github.com/user-attachments/assets/bb78c9ce-d655-49f6-8d34-99c12b8f35b3)

实验流程
1.根据上述要求，实现 HTTP 服务器程序

2.执行 sudo python topo.py 命令，生成包括两个端节点的网络拓扑

3.在主机 h1 上运行 HTTP 服务器程序，同时监听 80 和 443 端口 h1 # ./http-server 

4.在主机 h2 上运行测试程序，验证程序正确性 h2 # python3 test/test.py 如果没有出现 AssertionError 或其他错误，则说明程序实现正确

5.在主机 h1 上运行 http-server，所在目录下有一个小视频（30 秒左右）

6.在主机 h2 上运行 vlc（注意切换成普通用户），通过网络获取并播放该小视频媒体 -> 打开网络串流 -> 网络 -> 请输入网络 URL -> 播放

7.抓包，并对抓到的包进行解密

仅供参考学习。
