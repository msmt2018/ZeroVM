package dev.zerodroid.app
import android.app.Activity
import android.os.Bundle
import android.util.Log
import android.widget.TextView
import dev.zerodroid.core.nativebridge.NativeRuntime
class MainActivity : Activity() {
 override fun onCreate(savedInstanceState: Bundle?) {
  super.onCreate(savedInstanceState)
  val status = runCatching { "Native ABI ${NativeRuntime.abiVersion()}" }
   .getOrElse { error ->
    Log.e("ZeroDroid", "Native runtime unavailable", error)
    "Native runtime unavailable: ${error.javaClass.simpleName}"
   }
  setContentView(TextView(this).apply {
   text = "ZeroDroid\nApplication shell initialized\n$status"; textSize = 18f; setPadding(32,48,32,32)
  })
 }
}
