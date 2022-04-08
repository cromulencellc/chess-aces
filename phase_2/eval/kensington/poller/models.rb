require 'active_record'

class UserCheatsheet < ActiveRecord::Base
  self.table_name = 'users_cheatsheet'

  def self.sample
    self.order('random()').take
  end
end
