package org.chromium.chrome.browser.wootzapp_search;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.chrome.R;

import java.util.List;

/** Adapter for displaying chat messages. */
public class ChatAdapter extends RecyclerView.Adapter<ChatAdapter.ChatViewHolder> {
    private static final int VIEW_TYPE_USER = 1;
    private static final int VIEW_TYPE_AI = 2;
    private static final int VIEW_TYPE_ERROR = 3;
    private static final int VIEW_TYPE_LOADING = 4;

    private final List<ChatMessage> mMessages;

    public ChatAdapter(List<ChatMessage> messages) {
        mMessages = messages;
    }

    @NonNull
    @Override
    public ChatViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        int layoutId;
        switch (viewType) {
            case VIEW_TYPE_USER:
                layoutId = R.layout.chat_message_user;
                break;
            case VIEW_TYPE_ERROR:
                layoutId = R.layout.chat_message_ai;
                break;
            case VIEW_TYPE_LOADING:
                layoutId = R.layout.chat_message_ai;
                break;
            default:
                layoutId = R.layout.chat_message_ai;
                break;
        }
        View view = LayoutInflater.from(parent.getContext()).inflate(layoutId, parent, false);
        return new ChatViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ChatViewHolder holder, int position) {
        ChatMessage message = mMessages.get(position);
        holder.messageText.setText(message.getMessage());
        
        // Set different text colors based on message type
        if (message.isError()) {
            holder.messageText.setTextColor(holder.itemView.getContext().getResources().getColor(android.R.color.holo_red_dark));
        } else if (message.isLoading()) {
            holder.messageText.setTextColor(holder.itemView.getContext().getResources().getColor(android.R.color.holo_blue_dark));
        } else {
            holder.messageText.setTextColor(holder.itemView.getContext().getResources().getColor(android.R.color.black));
        }
    }

    @Override
    public int getItemCount() {
        return mMessages.size();
    }

    @Override
    public int getItemViewType(int position) {
        ChatMessage message = mMessages.get(position);
        if (message.isUser()) {
            return VIEW_TYPE_USER;
        } else if (message.isError()) {
            return VIEW_TYPE_ERROR;
        } else if (message.isLoading()) {
            return VIEW_TYPE_LOADING;
        } else {
            return VIEW_TYPE_AI;
        }
    }

    static class ChatViewHolder extends RecyclerView.ViewHolder {
        TextView messageText;

        ChatViewHolder(View itemView) {
            super(itemView);
            messageText = itemView.findViewById(R.id.message_text);
        }
    }
}