<div class="row">
	<div class="col-lg-3 pull-right">
		<div class="input-group">
			<input type="text" class="form-control" placeholder="[[admin/manage/groups:search-placeholder]]" id="group-search">
			<span class="input-group-addon search-button"><i class="fa fa-search"></i></span>
		</div>
	</div>
</div>
<div class="row groups">
	<div class="col-xs-12">
		<table class="table table-striped groups-list">
			<thead>
				<tr>
					<th>[[admin/manage/groups:name]]</th>
					<th>[[admin/manage/groups:badge]]</th>
					<th>[[admin/manage/groups:properties]]</th>
					<th></th>
				</tr>
			</thead>
			<tbody>
				<!-- BEGIN groups -->
				<tr data-groupname="{groups.displayName}" data-name-encoded="{groups.nameEncoded}">
					<td>
						<a href="{config.relative_path}/admin/manage/groups/{groups.nameEncoded}">{groups.displayName}</a> ({groups.memberCount})
						<p class="description">{groups.description}</p>
					</td>
					<td>
						<span class="label label-default" style="color:{groups.textColor}; background-color: {groups.labelColor};"><!-- IF groups.icon --><i class="fa {groups.icon}"></i> <!-- ENDIF groups.icon -->{groups.userTitle}</span>
					</td>
					<td>
						<!-- IF groups.system -->
						<span class="label label-danger">[[admin/manage/groups:system]]</span>
						<!-- ENDIF groups.system -->
						<!-- IF groups.private -->
						<span class="label label-primary">[[admin/manage/groups:private]]</span>
						<!-- ENDIF groups.private -->
						<!-- IF groups.hidden -->
						<span class="label label-default">[[admin/manage/groups:hidden]]</span>
						<!-- ENDIF groups.hidden -->
					</td>

					<td>
						<div class="btn-group">
							<a href="{config.relative_path}/api/admin/groups/{groups.nameEncoded}/csv" class="btn btn-default">[[admin/manage/groups:download-csv]]</a>

							<!-- IMPORT admin/partials/groups/privileges-select-category.tpl -->

							<!-- IF !groups.system -->
							<button class="btn btn-danger" data-action="delete"><i class="fa fa-times"></i></button>
							<!-- ENDIF !groups.system -->
						</div>
					</td>
				</tr>
				<!-- END groups -->
			</tbody>
			<tfoot>
				<tr>
					<td colspan="6"><br /><br /></td>
				</tr>
			</tfoot>
		</table>

		<!-- IMPORT partials/paginator.tpl -->
	</div>

	<div class="modal fade" id="create-modal">
		<div class="modal-dialog">
			<div class="modal-content">
				<div class="modal-header">
					<button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
					<h4 class="modal-title">[[admin/manage/groups:create]]</h4>
				</div>
				<div class="modal-body">
					<div class="alert alert-danger hide" id="create-modal-error"></div>
					<form>
						<div class="form-group">
							<label for="create-group-name">[[admin/manage/groups:name]]</label>
							<input type="text" class="form-control" id="create-group-name" placeholder="[[admin/manage/groups:name]]" />
						</div>
						<div class="form-group">
							<label for="create-group-desc">[[admin/manage/groups:description]]</label>
							<input type="text" class="form-control" id="create-group-desc" placeholder="[[admin/manage/groups:description-placeholder]]" />
						</div>
						<div class="form-group">
							<label>
								<input id="create-group-private" name="private" type="checkbox" checked>
								<strong>[[admin/manage/groups:private]]</strong>
							</label>
						</div>
						<div class="form-group">
							<label>
								<input id="create-group-hidden" name="hidden" type="checkbox">
								<strong>[[admin/manage/groups:hidden]]</strong>
							</label>
						</div>

					</form>
				</div>
				<div class="modal-footer">
					<button type="button" class="btn btn-default" data-dismiss="modal">
						[[global:close]]
					</button>
					<button type="button" class="btn btn-primary" id="create-modal-go">
						[[admin/manage/groups:create-button]]
					</button>
				</div>
			</div>
		</div>
	</div>
</div>

<button id="create" class="floating-button mdl-button mdl-js-button mdl-button--fab mdl-js-ripple-effect mdl-button--colored">
    <i class="material-icons">add</i>
</button>
