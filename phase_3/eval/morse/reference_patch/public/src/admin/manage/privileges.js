'use strict';

define('admin/manage/privileges', [
	'api',
	'autocomplete',
	'bootbox',
	'alerts',
	'translator',
	'categorySelector',
	'mousetrap',
	'admin/modules/checkboxRowSelector',
], function (api, autocomplete, bootbox, alerts, translator, categorySelector, mousetrap, checkboxRowSelector) {
	const Privileges = {};

	let cid;
	// number of columns to skip in category privilege tables
	const SKIP_PRIV_COLS = 3;

	Privileges.init = function () {
		cid = isNaN(parseInt(ajaxify.data.selectedCategory.cid, 10)) ? 'admin' : ajaxify.data.selectedCategory.cid;

		checkboxRowSelector.init('.privilege-table-container');

		categorySelector.init($('[component="category-selector"]'), {
			onSelect: function (category) {
				cid = parseInt(category.cid, 10);
				cid = isNaN(cid) ? 'admin' : cid;
				Privileges.refreshPrivilegeTable();
				ajaxify.updateHistory('admin/manage/privileges/' + (cid || ''));
			},
			localCategories: ajaxify.data.categories,
			privilege: 'find',
			showLinks: true,
		});

		Privileges.setupPrivilegeTable();

		highlightRow();
		$('.privilege-filters button:last-child').click();
	};

	Privileges.setupPrivilegeTable = function () {
		$('.privilege-table-container').on('change', 'input[type="checkbox"]:not(.checkbox-helper)', function () {
			const $checkboxEl = $(this);
			const $wrapperEl = $checkboxEl.parent();
			const columnNo = $wrapperEl.index() + 1;
			const privilege = $wrapperEl.attr('data-privilege');
			const state = $checkboxEl.prop('checked');
			const $rowEl = $checkboxEl.parents('tr');
			const member = $rowEl.attr('data-group-name') || $rowEl.attr('data-uid');
			const isPrivate = parseInt($rowEl.attr('data-private') || 0, 10);
			const isGroup = $rowEl.attr('data-group-name') !== undefined;
			const isBanned = (isGroup && $rowEl.attr('data-group-name') === 'banned-users') || $rowEl.attr('data-banned') !== undefined;
			const sourceGroupName = isBanned ? 'banned-users' : 'registered-users';
			const delta = $checkboxEl.prop('checked') === ($wrapperEl.attr('data-value') === 'true') ? null : state;

			if (member) {
				if (isGroup && privilege === 'groups:moderate' && !isPrivate && state) {
					bootbox.confirm('[[admin/manage/privileges:alert.confirm-moderate]]', function (confirm) {
						if (confirm) {
							$wrapperEl.attr('data-delta', delta);
							Privileges.exposeSingleAssumedPriv(columnNo, sourceGroupName);
						} else {
							$checkboxEl.prop('checked', !$checkboxEl.prop('checked'));
						}
					});
				} else if (privilege.endsWith('admin:admins-mods') && state) {
					bootbox.confirm('[[admin/manage/privileges:alert.confirm-admins-mods]]', function (confirm) {
						if (confirm) {
							$wrapperEl.attr('data-delta', delta);
							Privileges.exposeSingleAssumedPriv(columnNo, sourceGroupName);
						} else {
							$checkboxEl.prop('checked', !$checkboxEl.prop('checked'));
						}
					});
				} else {
					$wrapperEl.attr('data-delta', delta);
					Privileges.exposeSingleAssumedPriv(columnNo, sourceGroupName);
				}
				checkboxRowSelector.updateState($checkboxEl);
			} else {
				alerts.error('[[error:invalid-data]]');
			}
		});

		Privileges.exposeAssumedPrivileges();
		checkboxRowSelector.updateAll();
		Privileges.addEvents(); // events with confirmation modals
	};

	Privileges.addEvents = function () {
		document.getElementById('save').addEventListener('click', function () {
			throwConfirmModal('save', Privileges.commit);
		});

		document.getElementById('discard').addEventListener('click', function () {
			throwConfirmModal('discard', Privileges.discard);
		});

		// Expose discard button as necessary
		const containerEl = document.querySelector('.privilege-table-container');
		containerEl.addEventListener('change', (e) => {
			const subselector = e.target.closest('td[data-privilege] input');
			if (subselector) {
				document.getElementById('discard').style.display = containerEl.querySelectorAll('td[data-delta]').length ? 'unset' : 'none';
			}
		});

		const $privTableCon = $('.privilege-table-container');
		$privTableCon.on('click', '[data-action="search.user"]', Privileges.addUserToPrivilegeTable);
		$privTableCon.on('click', '[data-action="search.group"]', Privileges.addGroupToPrivilegeTable);
		$privTableCon.on('click', '[data-action="copyToChildren"]', function () {
			throwConfirmModal('copyToChildren', Privileges.copyPrivilegesToChildren.bind(null, cid, ''));
		});
		$privTableCon.on('click', '[data-action="copyToChildrenGroup"]', function () {
			const groupName = $(this).parents('[data-group-name]').attr('data-group-name');
			throwConfirmModal('copyToChildrenGroup', Privileges.copyPrivilegesToChildren.bind(null, cid, groupName));
		});

		$privTableCon.on('click', '[data-action="copyPrivilegesFrom"]', function () {
			Privileges.copyPrivilegesFromCategory(cid, '');
		});
		$privTableCon.on('click', '[data-action="copyPrivilegesFromGroup"]', function () {
			const groupName = $(this).parents('[data-group-name]').attr('data-group-name');
			Privileges.copyPrivilegesFromCategory(cid, groupName);
		});

		$privTableCon.on('click', '[data-action="copyToAll"]', function () {
			throwConfirmModal('copyToAll', Privileges.copyPrivilegesToAllCategories.bind(null, cid, ''));
		});
		$privTableCon.on('click', '[data-action="copyToAllGroup"]', function () {
			const groupName = $(this).parents('[data-group-name]').attr('data-group-name');
			throwConfirmModal('copyToAllGroup', Privileges.copyPrivilegesToAllCategories.bind(null, cid, groupName));
		});

		$privTableCon.on('click', '.privilege-filters > button', filterPrivileges);

		mousetrap.bind('ctrl+s', function (ev) {
			throwConfirmModal('save', Privileges.commit);
			ev.preventDefault();
		});

		function throwConfirmModal(method, onConfirm) {
			const privilegeSubset = getPrivilegeSubset();
			bootbox.confirm(`[[admin/manage/privileges:alert.confirm-${method}, ${privilegeSubset}]]<br /><br />[[admin/manage/privileges:alert.no-undo]]`, function (ok) {
				if (ok) {
					onConfirm.call();
				}
			});
		}
	};

	Privileges.commit = function () {
		const tableEl = document.querySelector('.privilege-table-container');
		const requests = $.map(tableEl.querySelectorAll('td[data-delta]'), function (el) {
			const privilege = el.getAttribute('data-privilege');
			const rowEl = el.parentNode;
			const member = rowEl.getAttribute('data-group-name') || rowEl.getAttribute('data-uid');
			const state = el.getAttribute('data-delta') === 'true' ? 1 : 0;

			return Privileges.setPrivilege(member, privilege, state);
		});

		Promise.allSettled(requests).then((results) => {
			Privileges.refreshPrivilegeTable();

			const rejects = results.filter(r => r.status === 'rejected');
			if (rejects.length) {
				rejects.forEach((result) => {
					alerts.error(result.reason);
				});
			} else {
				alerts.success('[[admin/manage/privileges:alert.saved]]');
			}
		});
	};

	Privileges.discard = function () {
		Privileges.refreshPrivilegeTable();
		alerts.success('[[admin/manage/privileges:alert.discarded]]');
	};

	Privileges.refreshPrivilegeTable = function (groupToHighlight) {
		api.get(`/categories/${cid}/privileges`, {}).then((privileges) => {
			ajaxify.data.privileges = { ...ajaxify.data.privileges, ...privileges };
			const tpl = parseInt(cid, 10) ? 'admin/partials/privileges/category' : 'admin/partials/privileges/global';
			const isAdminPriv = ajaxify.currentPage.endsWith('admin/manage/privileges/admin');
			app.parseAndTranslate(tpl, { privileges, isAdminPriv }).then((html) => {
				// Get currently selected filters
				const btnIndices = $('.privilege-filters button.btn-warning').map((idx, el) => $(el).index()).get();
				$('.privilege-table-container').html(html);
				Privileges.exposeAssumedPrivileges();
				document.querySelectorAll('.privilege-filters').forEach((con, i) => {
					// Three buttons, placed in reverse order
					const lastIdx = $('.privilege-filters').first().find('button').length - 1;
					const idx = btnIndices[i] === undefined ? lastIdx : btnIndices[i];
					con.querySelectorAll('button')[idx].click();
				});

				hightlightRowByDataAttr('data-group-name', groupToHighlight);
			});
		}).catch(alert.error);
	};

	Privileges.exposeAssumedPrivileges = function () {
		/*
			If registered-users has a privilege enabled, then all users and groups of that privilege
			should be assumed to have that privilege as well, even if not set in the db, so reflect
			this arrangement in the table
		*/

		// As such, individual banned users inherits privileges from banned-users group
		const getBannedUsersInputSelector = (privs, i) => `.privilege-table tr[data-banned] td[data-privilege="${privs[i]}"] input`;
		const bannedUsersPrivs = getPrivilegesFromRow('banned-users');
		applyPrivileges(bannedUsersPrivs, getBannedUsersInputSelector);

		// For rest that inherits from registered-users
		const getRegisteredUsersInputSelector = (privs, i) => `.privilege-table tr[data-group-name]:not([data-group-name="registered-users"],[data-group-name="banned-users"],[data-group-name="guests"],[data-group-name="spiders"]) td[data-privilege="${privs[i]}"] input, .privilege-table tr[data-uid]:not([data-banned]) td[data-privilege="${privs[i]}"] input`;
		const registeredUsersPrivs = getPrivilegesFromRow('registered-users');
		applyPrivileges(registeredUsersPrivs, getRegisteredUsersInputSelector);
	};

	Privileges.exposeSingleAssumedPriv = function (columnNo, sourceGroupName) {
		let inputSelectorFn;
		switch (sourceGroupName) {
			case 'banned-users':
				inputSelectorFn = () => `.privilege-table tr[data-banned] td[data-privilege]:nth-child(${columnNo}) input`;
				break;
			default:
				inputSelectorFn = () => `.privilege-table tr[data-group-name]:not([data-group-name="registered-users"],[data-group-name="banned-users"],[data-group-name="guests"],[data-group-name="spiders"]) td[data-privilege]:nth-child(${columnNo}) input, .privilege-table tr[data-uid]:not([data-banned]) td[data-privilege]:nth-child(${columnNo}) input`;
		}

		const sourceChecked = getPrivilegeFromColumn(sourceGroupName, columnNo);
		applyPrivilegesToColumn(inputSelectorFn, sourceChecked);
	};

	Privileges.setPrivilege = (member, privilege, state) => api[state ? 'put' : 'delete'](`/categories/${isNaN(cid) ? 0 : cid}/privileges/${privilege}`, { member });

	Privileges.addUserToPrivilegeTable = function () {
		const modal = bootbox.dialog({
			title: '[[admin/manage/categories:alert.find-user]]',
			message: '<input class="form-control input-lg" placeholder="[[admin/manage/categories:alert.user-search]]" />',
			show: true,
		});

		modal.on('shown.bs.modal', function () {
			const inputEl = modal.find('input');
			inputEl.focus();

			autocomplete.user(inputEl, function (ev, ui) {
				addUserToCategory(ui.item.user, function () {
					modal.modal('hide');
				});
			});
		});
	};

	Privileges.addGroupToPrivilegeTable = function () {
		const modal = bootbox.dialog({
			title: '[[admin/manage/categories:alert.find-group]]',
			message: '<input class="form-control input-lg" placeholder="[[admin/manage/categories:alert.group-search]]" />',
			show: true,
		});

		modal.on('shown.bs.modal', function () {
			const inputEl = modal.find('input');
			inputEl.focus();

			autocomplete.group(inputEl, function (ev, ui) {
				if (ui.item.group.name === 'administrators') {
					return alerts.alert({
						type: 'warning',
						message: '[[admin/manage/privileges:alert.admin-warning]]',
					});
				}
				addGroupToCategory(ui.item.group.name, function () {
					modal.modal('hide');
				});
			});
		});
	};

	Privileges.copyPrivilegesToChildren = function (cid, group) {
		const filter = getPrivilegeFilter();
		socket.emit('admin.categories.copyPrivilegesToChildren', { cid, group, filter }, function (err) {
			if (err) {
				return alerts.error(err.message);
			}
			alerts.success('[[admin/manage/categories:privileges.copy-success]]');
		});
	};

	Privileges.copyPrivilegesFromCategory = function (cid, group) {
		const privilegeSubset = getPrivilegeSubset();
		const message = '<br>' +
			(group ? `[[admin/manage/privileges:alert.copyPrivilegesFromGroup-warning, ${privilegeSubset}]]` :
				`[[admin/manage/privileges:alert.copyPrivilegesFrom-warning, ${privilegeSubset}]]`) +
			'<br><br>[[admin/manage/privileges:alert.no-undo]]';
		categorySelector.modal({
			title: '[[admin/manage/privileges:alert.copyPrivilegesFrom-title]]',
			message,
			localCategories: [],
			showLinks: true,
			onSubmit: function (selectedCategory) {
				socket.emit('admin.categories.copyPrivilegesFrom', {
					toCid: cid,
					filter: getPrivilegeFilter(),
					fromCid: selectedCategory.cid,
					group: group,
				}, function (err) {
					if (err) {
						return alerts.error(err);
					}
					ajaxify.refresh();
				});
			},
		});
	};

	Privileges.copyPrivilegesToAllCategories = function (cid, group) {
		const filter = getPrivilegeFilter();
		socket.emit('admin.categories.copyPrivilegesToAllCategories', { cid, group, filter }, function (err) {
			if (err) {
				return alerts.error(err);
			}
			alerts.success('[[admin/manage/categories:privileges.copy-success]]');
		});
	};

	function getPrivilegesFromRow(sourceGroupName) {
		const privs = [];
		$(`.privilege-table tr[data-group-name="${sourceGroupName}"] td input[type="checkbox"]:not(.checkbox-helper)`)
			.parent()
			.each(function (idx, el) {
				if ($(el).find('input').prop('checked')) {
					privs.push(el.getAttribute('data-privilege'));
				}
			});

		// Also apply to non-group privileges
		return privs.concat(privs.map(function (priv) {
			if (priv.startsWith('groups:')) {
				return priv.slice(7);
			}

			return false;
		})).filter(Boolean);
	}

	function getPrivilegeFromColumn(sourceGroupName, columnNo) {
		return $(`.privilege-table tr[data-group-name="${sourceGroupName}"] td:nth-child(${columnNo}) input[type="checkbox"]`)[0].checked;
	}

	function applyPrivileges(privs, inputSelectorFn) {
		for (let x = 0, numPrivs = privs.length; x < numPrivs; x += 1) {
			const inputs = $(inputSelectorFn(privs, x));
			inputs.each(function (idx, el) {
				if (!el.checked) {
					el.indeterminate = true;
				}
			});
		}
	}

	function applyPrivilegesToColumn(inputSelectorFn, sourceChecked) {
		const $inputs = $(inputSelectorFn());
		$inputs.each((idx, el) => {
			el.indeterminate = el.checked ? false : sourceChecked;
		});
	}

	function hightlightRowByDataAttr(attrName, attrValue) {
		if (attrValue) {
			const $el = $('[' + attrName + ']').filter(function () {
				return $(this).attr(attrName) === String(attrValue);
			});

			if ($el.length) {
				$el.addClass('selected');
				return true;
			}
		}
		return false;
	}

	function highlightRow() {
		if (ajaxify.data.group) {
			if (hightlightRowByDataAttr('data-group-name', ajaxify.data.group)) {
				return;
			}
			addGroupToCategory(ajaxify.data.group);
		}
	}

	function addGroupToCategory(group, cb) {
		cb = cb || function () {};
		const groupRow = document.querySelector('.privilege-table [data-group-name="' + group + '"]');
		if (groupRow) {
			hightlightRowByDataAttr('data-group-name', group);
			return cb();
		}
		// Generate data for new row
		const privilegeSet = ajaxify.data.privileges.keys.groups.reduce(function (memo, cur) {
			memo[cur] = false;
			return memo;
		}, {});

		app.parseAndTranslate('admin/partials/privileges/' + ((isNaN(cid) || cid === 0) ? 'global' : 'category'), 'privileges.groups', {
			privileges: {
				groups: [
					{
						name: group,
						nameEscaped: translator.escape(group),
						privileges: privilegeSet,
					},
				],
			},
		}, function (html) {
			const tbodyEl = document.querySelector('.privilege-table tbody');
			const btnIdx = $('.privilege-filters').first().find('button.btn-warning').index();
			tbodyEl.append(html.get(0));
			Privileges.exposeAssumedPrivileges();
			hightlightRowByDataAttr('data-group-name', group);
			document.querySelector('.privilege-filters').querySelectorAll('button')[btnIdx].click();
			cb();
		});
	}

	async function addUserToCategory(user, cb) {
		cb = cb || function () {};
		const userRow = document.querySelector('.privilege-table [data-uid="' + user.uid + '"]');
		if (userRow) {
			hightlightRowByDataAttr('data-uid', user.uid);
			return cb();
		}
		// Generate data for new row
		const privilegeSet = ajaxify.data.privileges.keys.users.reduce(function (memo, cur) {
			memo[cur] = false;
			return memo;
		}, {});

		const html = await app.parseAndTranslate('admin/partials/privileges/' + (isNaN(cid) ? 'global' : 'category'), 'privileges.users', {
			privileges: {
				users: [
					{
						picture: user.picture,
						username: user.username,
						banned: user.banned,
						uid: user.uid,
						'icon:text': user['icon:text'],
						'icon:bgColor': user['icon:bgColor'],
						privileges: privilegeSet,
					},
				],
			},
		});

		const tbodyEl = document.querySelectorAll('.privilege-table tbody');
		const btnIdx = $('.privilege-filters').last().find('button.btn-warning').index();
		tbodyEl[1].append(html.get(0));
		Privileges.exposeAssumedPrivileges();
		hightlightRowByDataAttr('data-uid', user.uid);
		document.querySelectorAll('.privilege-filters')[1].querySelectorAll('button')[btnIdx].click();
		cb();
	}

	function filterPrivileges(ev) {
		const [startIdx, endIdx] = ev.target.getAttribute('data-filter').split(',').map(i => parseInt(i, 10));
		const rows = $(ev.target).closest('table')[0].querySelectorAll('thead tr:last-child, tbody tr ');
		rows.forEach((tr) => {
			tr.querySelectorAll('td, th').forEach((el, idx) => {
				const offset = el.tagName.toUpperCase() === 'TH' ? 1 : 0;
				if (idx < (SKIP_PRIV_COLS - offset)) {
					return;
				}
				el.classList.toggle('hidden', !(idx >= (startIdx - offset) && idx <= (endIdx - offset)));
			});
		});
		checkboxRowSelector.updateAll();
		$(ev.target).siblings('button').toArray().forEach(btn => btn.classList.remove('btn-warning'));
		ev.target.classList.add('btn-warning');
	}

	function getPrivilegeFilter() {
		const indices = document.querySelector('.privilege-filters .btn-warning')
			.getAttribute('data-filter')
			.split(',')
			.map(i => parseInt(i, 10));
		indices[0] -= SKIP_PRIV_COLS;
		indices[1] = indices[1] - SKIP_PRIV_COLS + 1;
		return indices;
	}

	function getPrivilegeSubset() {
		const currentPrivFilter = document.querySelector('.privilege-filters .btn-warning');
		const filterText = currentPrivFilter ? currentPrivFilter.textContent.toLocaleLowerCase() : '';
		return filterText.indexOf('privileges') > -1 ? filterText : `${filterText} privileges`.trim();
	}

	return Privileges;
});
