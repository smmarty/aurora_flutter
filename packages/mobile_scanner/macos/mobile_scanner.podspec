#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint mobile_scanner.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'mobile_scanner'
  s.version          = '5.1.1'
  s.summary          = 'An universal scanner for Flutter based on MLKit.'
  s.description      = <<-DESC
An universal scanner for Flutter based on MLKit.
                       DESC
  s.homepage         = 'https://github.com/juliansteenbakker/mobile_scanner'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Julian Steenbakker' => 'juliansteenbakker@outlook.com' }
  s.source           = { :path => '.' }
  s.source_files     = 'Classes/**/*'
  s.dependency 'FlutterMacOS'
  s.platform = :osx, '10.14'
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES' }
  s.swift_version = '5.0'
end
