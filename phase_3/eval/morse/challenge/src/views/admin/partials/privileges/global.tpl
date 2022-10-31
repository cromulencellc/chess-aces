					<label>[[admin/manage/privileges:group-privileges]]</label>
					<table class="table table-striped privilege-table">
						<thead>
							{{{ if !isAdminPriv }}}
							<tr class="privilege-table-header">
								<th class="privilege-filters btn-toolbar" colspan="100">
									<!-- IF privileges.columnCountGroupOther -->
									<button type="button" data-filter="19,99" class="btn btn-default pull-right">[[admin/manage/categories:privileges.section-other]]</button>
									<!-- END -->
									<button type="button" data-filter="16,18" class="btn btn-default pull-right">[[admin/manage/categories:privileges.section-moderation]]</button>
									<button type="button" data-filter="3,8" class="btn btn-default pull-right">[[admin/manage/categories:privileges.section-posting]]</button>
									<button type="button" data-filter="9,15" class="btn btn-default pull-right">[[admin/manage/categories:privileges.section-viewing]]</button>
								</th>
							</tr><tr><!-- zebrastripe reset --></tr>
							{{{ end }}}
							<tr>
								<th colspan="2">[[admin/manage/categories:privileges.section-group]]</th>
								<th class="text-center">[[admin/manage/privileges:select-clear-all]]</th>
								<!-- BEGIN privileges.labels.groups -->
								<th class="text-center">{privileges.labels.groups.name}</th>
								<!-- END privileges.labels.groups -->
							</tr>
						</thead>
						<tbody>
							<!-- BEGIN privileges.groups -->
							<tr data-group-name="{privileges.groups.nameEscaped}" data-private="<!-- IF privileges.groups.isPrivate -->1<!-- ELSE -->0<!-- ENDIF privileges.groups.isPrivate -->">
								<td>
									{{{ if privileges.groups.isPrivate }}}
										{{{ if (privileges.groups.name == "banned-users") }}}
										<i class="fa fa-fw fa-exclamation-triangle text-muted" title="[[admin/manage/categories:privileges.inheritance-exception]]"></i>
										{{{ else }}}
										<i class="fa fa-fw fa-lock text-muted" title="[[admin/manage/categories:privileges.group-private]]"></i>
										{{{ end }}}
									{{{ else }}}
									<i class="fa fa-fw fa-none"></i>
									{{{ end }}}
									{privileges.groups.name}
								</td>
								<td></td>
								<td class="text-center"><input autocomplete="off" type="checkbox" class="checkbox-helper"></td>
								{function.spawnPrivilegeStates, privileges.groups.name, ../privileges}
							</tr>
							<!-- END privileges.groups -->
						</tbody>
						<tfoot>
							<tr>
								<td colspan="3"></td>
								<td colspan="{privileges.keys.groups.length}">
									<div class="btn-toolbar">
										<button type="button" class="btn btn-default pull-right" data-ajaxify="false" data-action="search.group">
											<i class="fa fa-users"></i>
											[[admin/manage/categories:privileges.search-group]]
										</button>
									</div>
								</td>
							</tr>
						</tfoot>
					</table>
					<div class="help-block">
						[[admin/manage/categories:privileges.inherit]]
					</div>
					<hr/>
					<label>[[admin/manage/privileges:user-privileges]]</label>
					<table class="table table-striped privilege-table">
						<thead>
							{{{ if !isAdminPriv }}}
							<tr class="privilege-table-header">
								<th class="privilege-filters btn-toolbar" colspan="100">
									<!-- IF privileges.columnCountUserOther -->
									<button type="button" data-filter="19,99" class="btn btn-default pull-right">[[admin/manage/categories:privileges.section-other]]</button>
									<!-- END -->
									<button type="button" data-filter="16,18" class="btn btn-default pull-right">[[admin/manage/categories:privileges.section-moderation]]</button>
									<button type="button" data-filter="3,8" class="btn btn-default pull-right">[[admin/manage/categories:privileges.section-posting]]</button>
									<button type="button" data-filter="9,15" class="btn btn-default pull-right">[[admin/manage/categories:privileges.section-viewing]]</button>
								</th>
							</tr><tr><!-- zebrastripe reset --></tr>
							{{{ end }}}
							<tr>
								<th colspan="2">[[admin/manage/categories:privileges.section-user]]</th>
								<th class="text-center">[[admin/manage/privileges:select-clear-all]]</th>
								<!-- BEGIN privileges.labels.users -->
								<th class="text-center">{privileges.labels.users.name}</th>
								<!-- END privileges.labels.users -->
							</tr>
						</thead>
						<tbody>
							<!-- BEGIN privileges.users -->
							<tr data-uid="{privileges.users.uid}"{{{ if privileges.users.banned }}} data-banned{{{ end }}}>
								<td>
									<!-- IF ../picture -->
									<img class="avatar avatar-sm" src="{privileges.users.picture}" title="{privileges.users.username}" alt="" />
									<!-- ELSE -->
									<div class="avatar avatar-sm" style="background-color: {../icon:bgColor};">{../icon:text}</div>
									<!-- ENDIF ../picture -->
								</td>
								<td>
									{{{ if privileges.users.banned }}}
										<i class="ban fa fa-gavel text-danger" title="[[admin/manage/categories:privileges.banned-user-inheritance]]"></i>
									{{{ end }}}
									{privileges.users.username}
								</td>
								<td class="text-center"><input autocomplete="off" type="checkbox" class="checkbox-helper"></td>
								{function.spawnPrivilegeStates, privileges.users.username, ../privileges}
							</tr>
							<!-- END privileges.users -->
						</tbody>
						<tfoot>
							<tr>
								<td colspan="3"></td>
								<td colspan="{privileges.keys.users.length}">
									<button type="button" class="btn btn-default pull-right" data-ajaxify="false" data-action="search.user">
										<i class="fa fa-user"></i>
										[[admin/manage/categories:privileges.search-user]]
									</button>
								</td>
							</tr>
						</tfoot>
					</table>
