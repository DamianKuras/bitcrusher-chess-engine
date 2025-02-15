-- Helper function: ensures a dependency is present by downloading and extracting it if needed
function ensure_dependency(name, url, extracted_dir)
    if not os.isdir(extracted_dir) then
        print("Fetching " .. name .. " dependency...")
        
        local zipFile = name .. ".zip"
        print("Downloading from " .. url)
        local _, status_code  = http.download(url, zipFile)

        if status_code  ~= 200 then
            error("Failed to download " .. name .. " (HTTP " .. http_code .. ")")
        end

        print("Extracting " .. name .. " to: " .. DEPS_DIR)
        zip.extract(zipFile, DEPS_DIR)
        os.remove(zipFile)

        if not os.isdir(extracted_dir) then
            print(extract)
            error("Extracted dependency structure mismatch for " .. name)
        end
       
    end
end