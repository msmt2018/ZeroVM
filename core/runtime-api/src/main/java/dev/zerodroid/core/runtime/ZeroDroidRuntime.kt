package dev.zerodroid.core.runtime
import dev.zerodroid.core.common.ZeroDroidResult
import dev.zerodroid.core.model.VmId
import dev.zerodroid.core.model.VmSpec
import dev.zerodroid.core.model.VmState
import kotlinx.coroutines.flow.Flow
data class VmSnapshot(val id:VmId,val state:VmState,val stateRevision:Long,val backendId:String?)
data class VmEvent(val vmId:VmId,val sequence:Long,val type:String,val timestampEpochMillis:Long,val operationId:String?)
data class VmOperation(val id:String,val vmId:VmId,val kind:Kind,val acceptedAtEpochMillis:Long) { enum class Kind { CREATE,START,SHUTDOWN,STOP,DESTROY } }
/**
 * Owns VM operations and event streams for one runtime instance.
 *
 * [close] is synchronous and non-blocking: it immediately prevents new operations,
 * cancels runtime-owned work, and completes active event flows. It does not wait for
 * non-cancellable backend I/O or guest shutdown to finish; late backend completions
 * must not publish events or mutate this closed runtime. Calling [close] more than
 * once is safe and has no additional effect.
 */
interface ZeroDroidRuntime:AutoCloseable {
 suspend fun create(spec:VmSpec):ZeroDroidResult<VmOperation>
 suspend fun start(id:VmId):ZeroDroidResult<VmOperation>
 suspend fun requestShutdown(id:VmId):ZeroDroidResult<VmOperation>
 suspend fun stop(id:VmId):ZeroDroidResult<VmOperation>
 suspend fun destroy(id:VmId):ZeroDroidResult<Unit>
 suspend fun snapshot(id:VmId):ZeroDroidResult<VmSnapshot>
 fun observeEvents(id:VmId):Flow<VmEvent>
 override fun close()
}
