import flash.utils.ByteArray;
import avmplus.System;


var a:ByteArray = new ByteArray();
a.position = 0;
print("hello" + a.position + "1\n");


for each (var file in System.argv)
{
     print(file + "\n");
}

