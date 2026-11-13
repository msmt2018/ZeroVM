plugins { alias(libs.plugins.android.library) }
android {
 namespace = "dev.zerovm.core.nativebridge"; compileSdk = 37; ndkVersion = "30.0.16248370"
 defaultConfig { minSdk = 28; externalNativeBuild { cmake { cppFlags += listOf("-std=c++20","-fvisibility=hidden") } } }
 externalNativeBuild { cmake { path = file("../../native/CMakeLists.txt") } }
}
dependencies { api(project(":core:common")) }
