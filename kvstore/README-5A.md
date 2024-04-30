Project 5A - Concurrent Store
====================

<!-- TODO: Fill this out. -->

## Design Overview: None.

## Collaborators: N/A

## Extra Credit Attempted: N/A

## Tweeter Reflection Questions:

### Q1.

<!-- My stakeholder pair was Sarsra Breisand and Beth Abraham (pair #2). -->

### Q2.

<!-- The delete that was implemented enables users to request all their posts to be deleted as well as their user profile. However, replies to their posts by other users as well as posts by other users that mention them have been maintained. This is because other Tweeter users have a right to freedom of speech (provided that their posts have not threatened, defamed, or otherwise consisted of non-protected speech about someone else). 

In this specific case, it was important to protect to strike a careful balance between protecting Sarsra Breisand's right to privacy and the right of other users' to their freedom of speech as well as the interests of other stakeholders. Deleting Sarsra's own posts and user ID made it harder to find personal information she shared and prevents her account from being accessed or otherwise linked to other data. While Sarsra's own content is removed to honor her request, completely erasing surrounding conversations would infringe on the free speech of other users who engaged with her posts. Keeping the replies and posts intact protects other users's self-epxression and protect their voices/perspectives from being silenced, while still supporting Sarsra's privacy interests. 

The design also does not go as far as eliminating every trace of Sarsra's existence across the platform. As a prominent public figure, information related to Sarsra has potential historical and research value. Stakeholders like historian Beth Abraham have an interest in studying social phenomena connected to Sarsra, and completely erasing such Sarsra's existence on Tweeter would hinder scholarly work and our collective understanding of these issues. Therefore, this approach was taken to preserve the ability to examine Sarsra's impact and legacy, even while significantly reducing access to her personal details. -->

### Q3. <!-- Q3A. If we assigned you only one stakeholder pair: -->

<!-- Tweeter's decision greatly limits the right to be forgotten. Sarsra Breisand might be displeased by our decision to leave posts that mention her and replies to her original posts, especially ones that may imply the location of her home. Although direct access to Sarsra's content is eliminated, the preservation of related discussions means that her involvement could still be indirectly discoverable, which may not fully achieve her goal of being "forgotten". On the other hand, researchers may have hope for greater retention of Sarsra's data. 

A generalized deletion mechanism would need to be responsive to the needs of a wide range of stakeholders. This includes individuals with sensitive personal histories who would prefer not to have their past brought back into public view. For example, ex-convicts who have served their sentences and are seeking to reintegrate into society may feel that continued visibility of their criminal records on social media platforms unduly hampers their rehabilitation. Individuals who have made bigoted or offensive comments in the past, perhaps in their youth or in a different social context, may wish to move beyond those statements and not have them continue to define their online presence. Others may simply have posted content that they now find embarrassing or shameful, and desire a fresh start. These individuals may be unsatisfied with out current deletion design and desire a more comprehensive deletion approach that removes a wider scope of personal data. However, again, their interests must be balanced against other important considerations, including freedom of expression and maintenance of historical records. Marginalized communities often rely on social media to document experiences of discrimination and build solidarity, and an overly aggressive deletion regime could undermine these important functions. 

In the long-term, our deletion design impacts the entire population. Tweeter makes it possible for anyone to share their views on nearly anything (provided it does not contain non-protected speech). With this decision, any user can share information about any person's past, and Tweeter will leave that post up. As a result, anyone who has made mistakes or done regrettable things will be unable to fully escape those actions. Tweeter recognized that we have all made mistakes and done things that we hope will be forgotten, and this decision makes that impossible. However, when weighing the competing interests at stake, Tweeter prioritizes freedom of speech and the open exchange of information. This choice may negatively impact some users or even non-users, but it upholds the fundamental right to free expression for all.

Considering the interests of other parties, such as future Tweeter users, ex-convicts, victims, and others would reinforce our decision. We believe that we have balanced the competing interests of both sides of this issue, as well as complying with GDPR. The interests of ex-convicts have been protected by allowing them to "correct the record" with their own posts and/or replies to others. The interests of victims have been been protected by allowing them to exercise their right to freedom of speech and post about the incident. Future Tweeter users' interests are protected since they are able to view all of those posts (from both sides) without restriction. Overall, there are few situations in which our policy would change based on the consideration of other stakeholders, since this policy was designed by weighing the interests of both the immediate and external stakeholders. -->

## Approximately how long did it take to complete the Concurrent Store portion of KVStore?

<!-- 25 hours -->

## Where would you rank the difficulty of the Concurrent Store portion of KVStore, compared to Snake, DMalloc, Caching I/O, and WeensyOS?

---Most difficult---
1. <br /> WeensyOS
2. <br /> Caching I/O
3. <br /> KVStore
4. <br /> Snake
5. <br /> DMalloc
---Least difficult---
