# Simple_Compiler
这是一个简单的minic编译器，minic语法可以参见说明文档。

编译器中间经过了三次变化，第一次从minic转为eeyore三地址码，第二次从eeyore转为tigger（实质上是分配了寄存器），第三次从tigger转为riscv汇编语言。

第一个部分最为麻烦，需要将繁杂的minic语句转为三地址码，具体可以在最里层的文件夹查看；第二部分需要进行活性分析并分配寄存器，可以在中层的文件夹查看；第三次只用做简单的一对一翻译工作，没有什么难度，可以在最外层的文件夹查看。

要使用该编译器，可以直接运行最外层文件夹的compile文件，将会在最外层生成三个可执行文件（eeyore、tigger和riscv），实际使用时可以把三者连接起来直接将minic一步翻译到汇编；当然也可以分开使用获得中间代码。
