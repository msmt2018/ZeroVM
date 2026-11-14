plugins { alias(libs.plugins.android.application) }
android {
 namespace = "dev.zerodroid.app"; compileSdk = 37; ndkVersion = "30.0.16248370"
 defaultConfig { applicationId = "dev.zerodroid.app"; minSdk = 28; targetSdk = 37; versionCode = 1; versionName = "0.1.0" }
 compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
}
dependencies {
 implementation(project(":core:common")); implementation(project(":core:model"))
 implementation(project(":core:runtime-api")); implementation(project(":core:native-bridge"))
}
