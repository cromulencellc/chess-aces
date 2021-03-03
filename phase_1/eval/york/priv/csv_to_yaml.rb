require 'csv'
require 'fileutils'
require 'pp'

base_data_dir = File.expand_path('../base_data', __dir__)
poller_dir = File.expand_path('../poller/data', __dir__)

FS = 0x1c # file
GS = 0x1d # group
RS = 0x1e # record
US = 0x1f # unit (cell)

FileUtils.mkdir_p base_data_dir
FileUtils.mkdir_p poller_dir

%w{orders items}.each do |fn|
  CSV.open(File.join(__dir__, "#{fn}.csv"), 'r:bom|utf-8') do |csv|
    File.open(File.join(base_data_dir, "#{fn}.yaml"), 'wb') do |asv|

      headers = csv.shift
      pp headers
      headers.each do |cell|
        asv.write cell
        asv.putc US
      end
      asv.putc GS

      csv.each do |row|
        pp row
        row.each do |cell|
          asv.write cell
          asv.putc US
        end
        asv.putc RS
      end
    end
  end

  FileUtils.cp(File.join(__dir__, "#{fn}.csv"), poller_dir)
end
