plugins { alias(libs.plugins.android.library) }
android { namespace = "dev.zerovm.core.model"; compileSdk = 37; defaultConfig { minSdk = 28 } }
dependencies { api(project(":core:common")) }
