这是一个基于protobuf序列化工具和muduo网络库，zookeeper配置中心开发的MPRPC框架。  
怎么使用：sudo ./autobuild.sh 先./zKService start来启动zookeeper service，然后到bin目录下先./provider -i test.config 然后./consumer -i test.config就可以运行example目录下的例子。  
通过阅读example目录下的例子可以更加清楚的知道如何调用该rpc框架。  
mprpc是怎么运行的  
