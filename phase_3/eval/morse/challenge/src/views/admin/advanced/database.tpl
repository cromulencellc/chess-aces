
<div class="row database">
	{{{ if mongo }}}
	<div class="col-lg-6">
		{{{ if mongo.serverStatusError }}}
		<div class="alert alert-warning">
			{mongo.serverStatusError}
		</div>
		{{{ end }}}
		<div class="panel panel-default">
			<div class="panel-heading"><i class="fa fa-hdd-o"></i> [[admin/advanced/database:mongo]]</div>
			<div class="panel-body">
				<div class="database-info">
					<span>[[admin/advanced/database:mongo.version]]</span> <span class="text-right">{mongo.version}</span><br/>
					<hr/>
					<span>[[admin/advanced/database:uptime-seconds]]</span> <span class="text-right formatted-number">{mongo.uptime}</span><br/>
					<span>[[admin/advanced/database:mongo.storage-engine]]</span> <span class="text-right">{mongo.storageEngine}</span><br/>
					<span>[[admin/advanced/database:mongo.collections]]</span> <span class="text-right formatted-number">{mongo.collections}</span><br/>
					<span>[[admin/advanced/database:mongo.objects]]</span> <span class="text-right formatted-number">{mongo.objects}</span><br/>
					<span>[[admin/advanced/database:mongo.avg-object-size]]</span> <span class="text-right">[[admin/advanced/database:x-b, {mongo.avgObjSize}]]</span><br/>
					<hr/>
					<span>[[admin/advanced/database:mongo.data-size]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.dataSize}]]</span><br/>
					<span>[[admin/advanced/database:mongo.storage-size]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.storageSize}]]</span><br/>
					<span>[[admin/advanced/database:mongo.index-size]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.indexSize}]]</span><br/>
					<!-- IF mongo.fileSize -->
					<span>[[admin/advanced/database:mongo.file-size]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.fileSize}]]</span><br/>
					<!-- ENDIF mongo.fileSize -->
					<hr/>
					<span>[[admin/advanced/database:mongo.resident-memory]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.mem.resident}]]</span><br/>
					<span>[[admin/advanced/database:mongo.virtual-memory]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.mem.virtual}]]</span><br/>
					<span>[[admin/advanced/database:mongo.mapped-memory]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.mem.mapped}]]</span><br/>
					<hr/>
					<span>[[admin/advanced/database:mongo.bytes-in]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.network.bytesIn}]]</span><br/>
					<span>[[admin/advanced/database:mongo.bytes-out]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {mongo.network.bytesOut}]]</span><br/>
					<span>[[admin/advanced/database:mongo.num-requests]]</span> <span class="text-right">{mongo.network.numRequests}</span><br/>
				</div>
			</div>
		</div>
	</div>
	{{{ end }}}

	{{{ if redis }}}
	<div class="col-lg-6">
		<div class="panel panel-default">
			<div class="panel-heading"><i class="fa fa-hdd-o"></i> [[admin/advanced/database:redis]]</div>
			<div class="panel-body">
				<div class="database-info">
					<span>[[admin/advanced/database:redis.version]]</span> <span class="text-right">{redis.redis_version}</span><br/>
					<hr/>
					<span>[[admin/advanced/database:uptime-seconds]]</span> <span class="text-right formatted-number">{redis.uptime_in_seconds}</span><br/>
					<span>[[admin/advanced/database:uptime-days]]</span> <span class="text-right">{redis.uptime_in_days}</span><br/>
					<hr/>
					<span>[[admin/advanced/database:redis.keys]]</span> <span class="text-right formatted-number">{redis.keys}</span><br/>
					<span>[[admin/advanced/database:redis.expires]]</span> <span class="text-right formatted-number">{redis.expires}</span><br/>
					<span>[[admin/advanced/database:redis.avg-ttl]]</span> <span class="text-right formatted-number">{redis.avg_ttl}</span><br/>
					<span>[[admin/advanced/database:redis.connected-clients]]</span> <span class="text-right">{redis.connected_clients}</span><br/>
					<span>[[admin/advanced/database:redis.connected-slaves]]</span> <span class="text-right">{redis.connected_slaves}</span><br/>
					<span>[[admin/advanced/database:redis.blocked-clients]]</span> <span class="text-right">{redis.blocked_clients}</span><br/>
					<hr/>

					<span>[[admin/advanced/database:redis.used-memory]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {redis.used_memory_human}]]</span><br/>
					<span>[[admin/advanced/database:redis.memory-frag-ratio]]</span> <span class="text-right">{redis.mem_fragmentation_ratio}</span><br/>
					<hr/>
					<span>[[admin/advanced/database:redis.total-connections-recieved]]</span> <span class="text-right formatted-number">{redis.total_connections_received}</span><br/>
					<span>[[admin/advanced/database:redis.total-commands-processed]]</span> <span class="text-right formatted-number">{redis.total_commands_processed}</span><br/>
					<span>[[admin/advanced/database:redis.iops]]</span> <span class="text-right formatted-number">{redis.instantaneous_ops_per_sec}</span><br/>

					<span>[[admin/advanced/database:redis.iinput]]</span> <span class="text-right">[[admin/advanced/database:x-mb, {redis.instantaneous_input}]]</span><br/>
					<span>[[admin/advanced/database:redis.ioutput]]</span> <span class="text-right">[[admin/advanced/database:x-mb, {redis.instantaneous_output}]]</span><br/>
					<span>[[admin/advanced/database:redis.total-input]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {redis.total_net_input}]]</span><br/>
					<span>[[admin/advanced/database:redis.total-output]]</span> <span class="text-right">[[admin/advanced/database:x-gb, {redis.total_net_output}]]</span><br/>

					<hr/>
					<span>[[admin/advanced/database:redis.keyspace-hits]]</span> <span class="text-right formatted-number">{redis.keyspace_hits}</span><br/>
					<span>[[admin/advanced/database:redis.keyspace-misses]]</span> <span class="text-right formatted-number">{redis.keyspace_misses}</span><br/>
				</div>
			</div>
		</div>
	</div>
	{{{ end }}}

	{{{ if postgres }}}
	<div class="col-lg-6">
		<div class="panel panel-default">
			<div class="panel-heading"><i class="fa fa-hdd-o"></i> [[admin/advanced/database:postgres]]</div>
			<div class="panel-body">
				<div class="database-info">
					<span>[[admin/advanced/database:postgres.version]]</span> <span class="text-right">{postgres.version}</span><br/>
					<hr/>
					<span>[[admin/advanced/database:uptime-seconds]]</span> <span class="text-right formatted-number">{postgres.uptime}</span><br/>
				</div>
			</div>
		</div>
	</div>
	{{{ end }}}
</div>

<div class="row database">
	{{{ if mongo }}}
	<div class="col-lg-6">
		<div class="panel panel-default">
			<div class="panel-heading" data-toggle="collapse" data-target=".mongodb-raw">
				<h3 class="panel-title"><i class="fa fa-caret-down"></i> [[admin/advanced/database:mongo.raw-info]]</h3>
			</div>

			<div class="panel-body mongodb-raw collapse">
				<div class="highlight">
					<pre>{mongo.raw}</pre>
				</div>
			</div>
		</div>
	</div>
	{{{ end }}}

	{{{ if redis }}}
	<div class="col-lg-6">
		<div class="panel panel-default">
			<div class="panel-heading" data-toggle="collapse" data-target=".redis-raw">
				<h3 class="panel-title"><i class="fa fa-caret-down"></i> [[admin/advanced/database:redis.raw-info]]</h3>
			</div>

			<div class="panel-body redis-raw collapse">
				<div class="highlight">
					<pre>{redis.raw}</pre>
				</div>
			</div>
		</div>
	</div>
	{{{ end }}}

	{{{ if postgres }}}
	<div class="col-lg-6">
		<div class="panel panel-default">
			<div class="panel-heading" data-toggle="collapse" data-target=".postgresql-raw">
				<h3 class="panel-title"><i class="fa fa-caret-down"></i> [[admin/advanced/database:postgres.raw-info]]</h3>
			</div>

			<div class="panel-body postgresql-raw collapse">
				<div class="highlight">
					<pre>{postgres.raw}</pre>
				</div>
			</div>
		</div>
	</div>
	{{{ end }}}
</div>
