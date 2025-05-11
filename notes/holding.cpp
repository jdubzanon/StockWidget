#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>

int main() {
    // Initialize libcurl globally.
    curl_global_init(CURL_GLOBAL_ALL);

    // Create a multi handle.
    CURLM* multi_handle = curl_multi_init();

    // List of URLs to fetch.
    std::vector<std::string> urls = {
        "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-quote-summary?symbol=AAPL&lang=en-US&region=US",
        "https://yahoo-finance-real-time1.p.rapidapi.com/stock/get-quote-summary?symbol=MSFT&lang=en-US&region=US"
    };

    // Create a list to hold individual easy handles.
    std::vector<CURL*> easy_handles;

    // Set up the HTTP headers required by RapidAPI.
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "x-rapidapi-key: d9e656a793mshfb457422b030792p1728dcjsn066cc8928220"); // Replace with your API key.
    headers = curl_slist_append(headers, "x-rapidapi-host: yahoo-finance-real-time1.p.rapidapi.com");
    // Optionally, add an Accept header to specify JSON:
    headers = curl_slist_append(headers, "Accept: application/json");

    // For each URL, create and configure an easy handle.
    for (const auto &url : urls) {
        CURL* easy_handle = curl_easy_init();
        if (!easy_handle) {
            std::cerr << "Failed to create easy handle." << std::endl;
            continue;
        }

        // Set the URL.
        curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
        // Attach the HTTP headers.
        curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);
        // Set the write destination to stdout (default write callback prints to stdout).
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, stdout);
        // Optional: Enable verbose mode for detailed debug output.
        curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);
        // Optionally, follow any redirects.
        curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
        // Set a timeout (in seconds) to avoid hanging.
        curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 10L);

        // Add this handle to the multi handle.
        curl_multi_add_handle(multi_handle, easy_handle);
        easy_handles.push_back(easy_handle);

        std::cout << "[main] Added request for: " << url << std::endl;
    }

    // Perform the multi request.
    int still_running = 0;
    CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
    if (mc != CURLM_OK) {
        std::cerr << "curl_multi_perform() failed: " << curl_multi_strerror(mc) << std::endl;
    }

    // Loop until all transfers are complete.
    while (still_running) {
        int numfds = 0;
        mc = curl_multi_wait(multi_handle, nullptr, 0, 1000, &numfds);
        if (mc != CURLM_OK) {
            std::cerr << "curl_multi_wait() failed: " << curl_multi_strerror(mc) << std::endl;
            break;
        }
        mc = curl_multi_perform(multi_handle, &still_running);
    }

    // Cleanup: Remove and clean up each easy handle.
    for (CURL* handle : easy_handles) {
        curl_multi_remove_handle(multi_handle, handle);
        curl_easy_cleanup(handle);
    }

    // Free the header list.
    curl_slist_free_all(headers);
    // Clean up the multi handle and global libcurl state.
    curl_multi_cleanup(multi_handle);
    curl_global_cleanup();

    return 0;
}





