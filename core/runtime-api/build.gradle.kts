plugins { alias(libs.plugins.android.library) }
android { namespace = "dev.zerovm.core.runtime"; compileSdk = 37; defaultConfig { minSdk = 28 } }
dependencies { api(project(":core:common")); api(project(":core:model")); api(libs.kotlinx.coroutines.core) }
