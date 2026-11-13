pluginManagement { repositories { google(); mavenCentral(); gradlePluginPortal() } }
dependencyResolutionManagement {
 repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
 repositories { google(); mavenCentral() }
}
rootProject.name = "ZeroVM"
include(":app", ":core:common", ":core:model", ":core:runtime-api", ":core:native-bridge")
