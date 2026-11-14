package dev.zerodroid.core.model
enum class VmState { CREATED,PREFLIGHT,PREPARED,STARTING,RUNNING,STOPPING,STOPPED,FAILED,DESTROYING,DESTROYED }
data class VmId(val value:String) { init { require(value.isNotBlank() && value.length<=128) { "VmId must contain 1..128 non-blank characters" } } }
data class GuestProfileRef(val profileId:String,val revision:String)
data class ResourceRange(val minimum:Long,val maximum:Long,val adaptive:Boolean) { init { require(minimum>=0L) { "ResourceRange.minimum must be non-negative" }; require(maximum>=minimum) { "ResourceRange.maximum must be greater than or equal to minimum" } } }
data class VmResourcePolicy(val vcpus:ResourceRange,val memoryBytes:ResourceRange,val storageQuotaBytes:Long) { init { require(storageQuotaBytes>0L) { "VmResourcePolicy.storageQuotaBytes must be positive" } } }
data class VmSpec(val id:VmId,val displayName:String,val guestProfile:GuestProfileRef,val imageSetId:String,val resources:VmResourcePolicy) {
 init { require(displayName.isNotBlank()) { "VmSpec.displayName must not be blank" }; require(imageSetId.isNotBlank()) { "VmSpec.imageSetId must not be blank" } }
}
